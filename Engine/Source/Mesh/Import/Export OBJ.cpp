/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Bool ExportOBJ(C Str &name, C MeshLod &mesh)
{
   FileText f; if(f.write(name, ANSI))
   {
      Int vp=1,
          vt=1,
          vn=1;
      FREPA(mesh)
      {
       C MeshPart &part=mesh.parts[i];
       C MeshBase *base=&part.base;
         MeshBase  temp; if(!base->is() && part.render.is())base=&temp.create(part.render); 
       C Vec      *pos =base->vtx.pos ();
       C Vec      *nrm =base->vtx.nrm ();
       C Vec2     *tex =base->vtx.tex0();
                                f.putLine(S+"g " +Replace(part.name, '\n', ' '));
         if(pos)FREPA(base->vtx)f.putLine(S+"v " +Dbl(-pos[i].x)+' '+Dbl(  pos[i].y)+' '+Dbl(pos[i].z));
         if(tex)FREPA(base->vtx)f.putLine(S+"vt "+Dbl( tex[i].x)+' '+Dbl(1-tex[i].y)                  );
         if(nrm)FREPA(base->vtx)f.putLine(S+"vn "+Dbl(-nrm[i].x)+' '+Dbl(  nrm[i].y)+' '+Dbl(nrm[i].z));

         if(C VecI *tri=base->tri.ind())FREPA(base->tri)
         {
            VecI t=tri[i]; t.reverse();
            f.putText(S+"f "+(t.x+vp)+'/'); if(tex)f.putText(S+(t.x+vt)); f.putChar('/'); if(nrm)f.putText(S+(t.x+vn));
            f.putText(S+ " "+(t.y+vp)+'/'); if(tex)f.putText(S+(t.y+vt)); f.putChar('/'); if(nrm)f.putText(S+(t.y+vn));
            f.putText(S+ " "+(t.z+vp)+'/'); if(tex)f.putText(S+(t.z+vt)); f.putChar('/'); if(nrm)f.putText(S+(t.z+vn));
            f.endLine();
         }
         if(C VecI4 *quad=base->quad.ind())FREPA(base->quad)
         {
            VecI4 q=quad[i]; q.reverse();
            f.putText(S+"f "+(q.x+vp)+'/'); if(tex)f.putText(S+(q.x+vt)); f.putChar('/'); if(nrm)f.putText(S+(q.x+vn));
            f.putText(S+ " "+(q.y+vp)+'/'); if(tex)f.putText(S+(q.y+vt)); f.putChar('/'); if(nrm)f.putText(S+(q.y+vn));
            f.putText(S+ " "+(q.z+vp)+'/'); if(tex)f.putText(S+(q.z+vt)); f.putChar('/'); if(nrm)f.putText(S+(q.z+vn));
            f.putText(S+ " "+(q.w+vp)+'/'); if(tex)f.putText(S+(q.w+vt)); f.putChar('/'); if(nrm)f.putText(S+(q.w+vn));
            f.endLine();
         }

         if(pos)vp+=base->vtxs();
         if(tex)vt+=base->vtxs();
         if(nrm)vn+=base->vtxs();
      }
      return true;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
