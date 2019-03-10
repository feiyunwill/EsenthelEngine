/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static const Vec2 nrm[3]=
{
   Vec2(Cos(PI2*0/3), Sin(PI2*0/3)),
   Vec2(Cos(PI2*1/3), Sin(PI2*1/3)),
   Vec2(Cos(PI2*2/3), Sin(PI2*2/3)),
};
/******************************************************************************/
void DrawVelocityBlur(Flt power, C Ball &ball)
{
   if(Renderer._vel)
   {
      Mtn.load();

      SetOneMatrix();
      Sh.h_Step->set(power);
   #if DX9
      D.colWrite(0); // disable color writes, we want to write only to velocity RT and not color RT, #BlendRT
   #endif
      D .depthWrite(false); Renderer.needDepthTest(); // !! 'needDepthTest' after 'depthWrite' !!
      D .alpha     (ALPHA_NONE);
      D .stencil   (STENCIL_NONE);
      VI.shader    (Mtn.h_Explosion);
      VI.cull      (true); ball.drawVI(true);
      VI.end       (    );
   #if DX9
      D.colWrite(COL_WRITE_RGBA); // restore color writes
   #endif
   }
}
/******************************************************************************/
struct LaserSegment
{
   Vec    p[3],
          n[3];
   Matrix m;
};
void DrawLaser(C Color &color, C Color &middle_color, Flt middle_exponent, Flt radius, Bool sharp_ending, C MemPtr<Vec> &points)
{
   if(Renderer.firstPass() && points.elms()>1)
   {
      Vec2 pnt[3]=
      {
         nrm[0]*radius,
         nrm[1]*radius,
         nrm[2]*radius,
      };
      LaserSegment s[2], *c=&s[0], *n=&s[1];

      c->m.setPosDir(points[0], !(points[1]-points[0]));

      Bool normals=(color!=middle_color && middle_exponent>EPS_GPU && Renderer()!=RM_AMBIENT),
           closed =Equal(points.first(), points.last());
      if(sharp_ending && points.elms()<3)sharp_ending=false;
      if(sharp_ending)
      {
         closed=false;
         c->p[0]=points[0];
         c->p[1]=points[0];
         c->p[2]=points[0];
         if(normals)
         {
            c->n[0]=-c->m.z;
            c->n[1]=-c->m.z;
            c->n[2]=-c->m.z;
         }
      }else
      {
         c->p[0]=c->m.convert(pnt[0]);
         c->p[1]=c->m.convert(pnt[1]);
         c->p[2]=c->m.convert(pnt[2]);
         if(normals)
         {
            if(closed)
            {
               c->n[0]=c->m.orn().convert(nrm[0]);
               c->n[1]=c->m.orn().convert(nrm[1]);
               c->n[2]=c->m.orn().convert(nrm[2]);
            }else
            {
               c->n[0]=!(c->m.orn().convert(nrm[0]) - c->m.z);
               c->n[1]=!(c->m.orn().convert(nrm[1]) - c->m.z);
               c->n[2]=!(c->m.orn().convert(nrm[2]) - c->m.z);
            }
         }
      }

      if(Renderer()==RM_AMBIENT)
      {
         VI.color (Color(255, 255, 255, 0));
         VI.color2(Color(255, 255, 255, 0));
      }else
      {
         VI.shader(Sh.h_Laser[normals]);
         VI.color (       color);
         VI.color2(middle_color);
         Sh.h_Step->set(middle_exponent);
         if(color.a || middle_color.a)Renderer._has_glow=true;
      }
      VI.cull   (true);
      VI.setType(normals ? VI_3D_LASER : VI_3D_FLAT);

      if(!closed)
      {
         if(normals)
         {
            if(Vtx3DLaser *v=(Vtx3DLaser*)VI.addVtx(3))
            {
               v[0].pos=c->p[2];
               v[1].pos=c->p[1];
               v[2].pos=c->p[0];

               v[0].nrm=c->n[2];
               v[1].nrm=c->n[1];
               v[2].nrm=c->n[0];
            }
         }else
         {
            if(Vtx3DFlat *v=(Vtx3DFlat*)VI.addVtx(3))
            {
               v[0].pos=c->p[2];
               v[1].pos=c->p[1];
               v[2].pos=c->p[0];
            }
         }
      }

      for(Int i=1; i<points.elms(); i++)
      {
         n->m.pos=points[i];
         if(i+1<points.elms())
         {
            n->m.z=!(points[i+1]-points[i]);
            n->m.y=PointOnPlane(c->m.y, n->m.z); if(!n->m.y.normalize())n->m.y=PerpN(n->m.z);
            n->m.x=Cross(n->m.y, n->m.z);
         }else
         {
            if(!closed)n->m.orn()=c->m.orn();else
            {
               n->m.orn().setDir(!(points[1]-points[0]));
            }
         }
         if(i==points.elms()-1 && sharp_ending)
         {
            n->p[0]=points[i];
            n->p[1]=points[i];
            n->p[2]=points[i];
            if(normals)
            {
               n->n[0]=c->m.z;
               n->n[1]=c->m.z;
               n->n[2]=c->m.z;
            }
         }else
         {
            n->p[0]=n->m.convert(pnt[0]);
            n->p[1]=n->m.convert(pnt[1]);
            n->p[2]=n->m.convert(pnt[2]);
            if(normals)
            {
               if(i==points.elms()-1 && !closed)
               {
                  n->n[0]=!(n->m.orn().convert(nrm[0]) + n->m.z);
                  n->n[1]=!(n->m.orn().convert(nrm[1]) + n->m.z);
                  n->n[2]=!(n->m.orn().convert(nrm[2]) + n->m.z);
               }else
               {
                  n->n[0]=n->m.orn().convert(nrm[0]);
                  n->n[1]=n->m.orn().convert(nrm[1]);
                  n->n[2]=n->m.orn().convert(nrm[2]);
               }
            }
         }

         if(normals)
         {
            if(Vtx3DLaser *v=(Vtx3DLaser*)VI.addVtx(6*3)) // 3 quads (quad=2*3 vertexes)
            {
               // 0-1 quad
               v[ 0].pos=c->p[0];
               v[ 1].pos=c->p[1];
               v[ 2].pos=n->p[1];
               v[ 3].pos=n->p[1];
               v[ 4].pos=n->p[0];
               v[ 5].pos=c->p[0];

               v[ 0].nrm=c->n[0];
               v[ 1].nrm=c->n[1];
               v[ 2].nrm=n->n[1];
               v[ 3].nrm=n->n[1];
               v[ 4].nrm=n->n[0];
               v[ 5].nrm=c->n[0];

               // 1-2 quad
               v[ 6].pos=c->p[1];
               v[ 7].pos=c->p[2];
               v[ 8].pos=n->p[2];
               v[ 9].pos=n->p[2];
               v[10].pos=n->p[1];
               v[11].pos=c->p[1];

               v[ 6].nrm=c->n[1];
               v[ 7].nrm=c->n[2];
               v[ 8].nrm=n->n[2];
               v[ 9].nrm=n->n[2];
               v[10].nrm=n->n[1];
               v[11].nrm=c->n[1];

               // 2-0 quad
               v[12].pos=c->p[2];
               v[13].pos=c->p[0];
               v[14].pos=n->p[0];
               v[15].pos=n->p[0];
               v[16].pos=n->p[2];
               v[17].pos=c->p[2];

               v[12].nrm=c->n[2];
               v[13].nrm=c->n[0];
               v[14].nrm=n->n[0];
               v[15].nrm=n->n[0];
               v[16].nrm=n->n[2];
               v[17].nrm=c->n[2];
            }
         }else
         {
            if(Vtx3DFlat *v=(Vtx3DFlat*)VI.addVtx(6*3)) // 3 quads (quad=2*3 vertexes)
            {
               // 0-1 quad
               v[ 0].pos=c->p[0];
               v[ 1].pos=c->p[1];
               v[ 2].pos=n->p[1];
               v[ 3].pos=n->p[1];
               v[ 4].pos=n->p[0];
               v[ 5].pos=c->p[0];

               // 1-2 quad
               v[ 6].pos=c->p[1];
               v[ 7].pos=c->p[2];
               v[ 8].pos=n->p[2];
               v[ 9].pos=n->p[2];
               v[10].pos=n->p[1];
               v[11].pos=c->p[1];

               // 2-0 quad
               v[12].pos=c->p[2];
               v[13].pos=c->p[0];
               v[14].pos=n->p[0];
               v[15].pos=n->p[0];
               v[16].pos=n->p[2];
               v[17].pos=c->p[2];
            }
         }

         Swap(n, c);
      }

      if(!closed)
      {
         if(normals)
         {
            if(Vtx3DLaser *v=(Vtx3DLaser*)VI.addVtx(3))
            {
               v[0].pos=c->p[0];
               v[1].pos=c->p[1];
               v[2].pos=c->p[2];

               v[0].nrm=c->n[0];
               v[1].nrm=c->n[1];
               v[2].nrm=c->n[2];
            }
         }else
         {
            if(Vtx3DFlat *v=(Vtx3DFlat*)VI.addVtx(3))
            {
               v[0].pos=c->p[0];
               v[1].pos=c->p[1];
               v[2].pos=c->p[2];
            }
         }
      }
      VI.end();
   }
}
/******************************************************************************/
ElectricityFx::ElectricityFx()
{
          color   .set(64, 96, 176);
   middle_color   =WHITE;
   middle_exponent=5;
   radius         =0.008f;
   time           =0;
   frequency      =0.03f;
   random_step    =0.1f;
   random_radius  =0.1f;
}
void ElectricityFx::update()
{
   time-=Time.d();
   if(time<=0)
   {
      time+=frequency; if(time<=0)time=frequency;

      randomized.clear();
      if(original.elms()>1)
      {
         Ball ball(random_radius);
         Vec  prev=randomized.New()=original.first();
         for(Int i=1; i<original.elms(); i++)
         {
            Vec next=original[i];
            for(;;)
            {
               Flt dist=Dist(prev, next);
               if( dist>random_step)
               {
                  if(i==original.elms()-1 && dist<random_step*1.5)break;
                  prev=Lerp(prev, next, random_step/dist);
                  randomized.New()=prev+Random(ball);
               }else break;
            }
         }
         randomized.New()=original.last();
      }
   }
}
void ElectricityFx::draw()
{
   DrawLaser(color, middle_color, middle_exponent, radius, false, randomized);
}
/******************************************************************************/
}
/******************************************************************************/
