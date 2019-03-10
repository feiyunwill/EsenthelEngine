/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
class TrailEdge
{
   Vtx3DSimple vtx[2];
}
class Trail // Swing Trail Effect
{
   Memc<TrailEdge> edges           ; // container of edges
   int             edge_offset=0   ; // index of the most recently updated edge
   flt             edge_time  =0   , // how long current edge is being updated
                   interval   =0.08; // time duration for updating one edge
   ImagePtr        image;

   void create(int edges)
   {
      T.edges.setNumZero(edges);
      image=UID(3286123862, 1332751955, 2940666549, 4213579942); // setup texture

      // prepare vertex Y texture coordinates (they will always remain the same)
      REPA(T.edges)
      {
         T.edges[i].vtx[0].tex.y=0;
         T.edges[i].vtx[1].tex.y=1;
      }
   }

   void update(C Vec &new_pos_start, C Vec &new_pos_end)
   {
      if(edges.elms())
      {
            edge_time+=Time.d(); // increase edge time counter
         if(edge_time>=interval) // if       edge has been updated for too long, then leave this edge, and start updating next edge
         {
            edge_time=0; // reset counter

            int old_edge=edge_offset;                 // remember old edge index
            edge_offset=(edge_offset+1)%edges.elms(); // increase current edge index

            // copy texture coordinates from last edge
            edges[edge_offset].vtx[0].tex.x=edges[old_edge].vtx[0].tex.x;
            edges[edge_offset].vtx[1].tex.x=edges[old_edge].vtx[1].tex.x;
         }

         // always update latest edge position and tex coords
         {
            // position
            edges[edge_offset].vtx[0].pos=new_pos_start;
            edges[edge_offset].vtx[1].pos=new_pos_end  ;

            // tex coords
            flt tex_progress=Time.d()*0.5;
            edges[edge_offset].vtx[0].tex.x+=tex_progress;
            edges[edge_offset].vtx[1].tex.x+=tex_progress;
         }

         // always update all edge colors
         flt alpha=1, alpha_step=1.0/flt(edges.elms()-2); // start from most recent edge, and set its alpha to full
         FREPA(edges)
         {
            int   index=Mod(edge_offset-i, edges.elms());
            Color color=ColorAlpha(Color(255, 255, 255, 128), alpha);
            edges[index].vtx[0].color=color;
            edges[index].vtx[1].color=color;
            if(i==0)alpha-=alpha_step*(edge_time/interval); // first step is not full, because the edge is only partially long
            else    alpha-=alpha_step;
         }
      }
   }

   void draw() // this will be called only in RM_BLEND mode
   {
      SetMatrix(); // set identity matrix

      // before drawing custom graphics in RM_BLEND using 'VI' we need to set alpha blending mode in order to avoid accidental glow effect
      D.alpha(ALPHA_BLEND_DEC);

      // use Vertex Index Buffer for custom drawing
    //VI.shader(..); setup custom shader if needed
      VI.image(image()); // set image
      VI.wrap (); // set wrap texture addressing mode
      REP(edges.elms()-1)
      {
         // get indexes of neighbour edges
         int next=Mod(edge_offset-i  , edges.elms()),
             prev=Mod(edge_offset-i-1, edges.elms());

         // draw quad face from 4 points
         VI.face(edges[prev].vtx[1],
                 edges[prev].vtx[0],
                 edges[next].vtx[0],
                 edges[next].vtx[1]);
      }
      VI.end();
   }
}
/******************************************************************************/
MeshPtr weapon;
Matrix  weapon_matrix;
Trail   trail;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   D.viewRange(15);
}
/******************************************************************************/
Bool Init()
{
   // camera
   Cam.dist=3;
   Cam.yaw=PI;
   Cam.pitch=-0.4;
   Cam.setSpherical().set();

   // sun
   Sun.image=UID(1275694243, 1199742097, 1108828586, 1055787228);
   Sun.pos  =!Vec(1, 1, 3);
   Sun.light_color=1-D.ambientColor();

   // sky
   Sky.atmospheric();

   // weapon
   weapon=ObjectPtr(UID(3865949516, 1110691029, 816879537, 746462228))->mesh();

   // trail effect
   trail.create(8);

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
Bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Gui.update();

   // update weapon motion
   weapon_matrix.setPosUp(Vec(0, 0, 0.5), Vec(0, 0, 1))
                .rotateY (Time.time())
                .move    (Vec(0, 0.3, 1.2));

   // update trail effect
   trail.update(weapon->ext.down()*weapon_matrix,
                weapon->ext.up  ()*weapon_matrix);

   // camera
   Cam.transformByMouse(0.01, 10, CAMH_ZOOM|CAMH_ROT);

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         weapon->draw(weapon_matrix);
      }break;

      case RM_BLEND:
      {
         trail.draw();
      }break;
   }
}
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
