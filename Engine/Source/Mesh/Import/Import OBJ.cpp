/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace OBJ{ // so local structures are unique
/******************************************************************************/
struct FVtx
{
   Int pos, nrm, tex;

   FVtx() {pos=nrm=tex=-1;}
};
struct FTri
{
   VecI pos, nrm, tex;
};
struct FQuad
{
   VecI4 pos, nrm, tex;
};
struct Group
{
   Memc<FTri > tri     ;
   Memc<FQuad> quad    ;
   Int         material;
   Str         name    ;

   Int faces()C {return tri.elms() + quad.elms();}

   Group& set(Int material) {T.material=material; return T;}
};
/******************************************************************************/
static void ImportMtl(C Str &name, MemPtr<XMaterial> materials)
{
   FileTextEx f; if(f.read(name))
   {
      for(Str line; !f.end(); )
      {
         f.getLine(line);
         if(Starts(line, "newmtl "  )){                    materials.New ().name        =        _SkipChar(TextPos(line, ' ')) ;}else
         if(Starts(line, "Ke "      )){if(materials.elms())materials.last().ambient     =TextVec(_SkipChar(TextPos(line, ' ')));}else
         if(Starts(line, "Kd "      )){if(materials.elms())materials.last().color.xyz   =TextVec(_SkipChar(TextPos(line, ' ')));}else
         if(Starts(line, "Ks "      )){if(materials.elms())materials.last().specular    =TextVec(_SkipChar(TextPos(line, ' ')));}else
         if(Starts(line, "map_Ke "  )){if(materials.elms())materials.last().   light_map=        _SkipChar(TextPos(line, ' ')) ;}else
         if(Starts(line, "map_Kd "  )){if(materials.elms())materials.last().   color_map=        _SkipChar(TextPos(line, ' ')) ;}else
         if(Starts(line, "map_Ks "  )){if(materials.elms())materials.last().specular_map=        _SkipChar(TextPos(line, ' ')) ;}else
         if(Starts(line, "map_bump "))
         {
            if(materials.elms())
            {
               Str name=_SkipChar(TextPos(line, ' ')); if(Starts(name, "-bm"))name=_SkipChar(TextPos(_SkipChar(TextPos(name, ' ')), ' '));
               materials.last().normal_map=name;
            }
         }
      }
      REPAO(materials).fixPath(GetPath(name));
   }
}
/******************************************************************************/
} // namespace OBJ
/******************************************************************************/
Bool ImportOBJ(C Str &name, Mesh *mesh, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index)
{
   if(mesh)mesh->del();
   materials          .clear();
   part_material_index.clear();

   using namespace OBJ;

   FileTextEx f; if(f.read(name))
   {
      Int         cur_material=-1;
      Bool        flip_normal_y=false;
      Memc<Vec  > vpos;
      Memc<Vec  > vnrm;
      Memc<Vec2 > vtex;
      Memc<Group> groups;
      Memc<FVtx > fvtx;

      for(Str line; !f.end(); )
      {
         Char c=f.getChar();
         if(c=='m' && f.getChar()=='t' && f.getChar()=='l' && f.getChar()=='l' && f.getChar()=='i' && f.getChar()=='b' && f.getChar()==' ')
         {
            if(materials)ImportMtl(S+_GetPath(name)+'/'+f.getLine(), materials);else f.skipLine();
         }else
         if(c=='v')
         {
            c=f.getChar();
            if(c==' '                    )f.get(vpos.New());else
            if(c=='n' && f.getChar()==' ')vnrm.add(!f.getVec());else
            if(c=='t' && f.getChar()==' '){Vec2 &t=vtex.New(); f.get(t); t.y=1-t.y;}
         }else
         if(c=='g' && f.getChar()==' ')
         {
            if(groups.elms() && !groups.last().faces())groups.removeLast();
            f.getLine(groups.New().set(cur_material).name);
         }else
         if(c=='u' && f.getChar()=='s' && f.getChar()=='e' && f.getChar()=='m' && f.getChar()=='t' && f.getChar()=='l' && f.getChar()==' ')
         {
            if(materials)
            {
               f.getLine(line);
               cur_material=-1; REPA(materials)if(Equal(materials[i].name, line)){cur_material=i; break;}
            }else f.skipLine();
            if(groups.elms() && groups.last().faces())groups.New ().set(cur_material);else
            if(groups.elms()                         )groups.last().set(cur_material);
         }else
         if(c=='f' && f.getChar()==' ')
         {
            if(!groups.elms())groups.New().set(cur_material);
            Group &group=groups.last();
            f.getLine(line);
            for(CChar *face=line; face; )
            {
               CalcValue cval; face=TextValue(face, cval); if(cval.type)
               {
                  FVtx &vtx=fvtx.New();
                  vtx.pos=cval.asInt()-1; if(!InRange(vtx.pos, vpos))goto invalid; // can't be invalid
                  if(Is(face) && *face++=='/')
                     if(*face!=' ') // space means proceed to next index
                  {
                     face=TextValue(face, cval); if(cval.type)vtx.tex=cval.asInt()-1; // can be invalid, because it's checked later
                     if(Is(face) && *face++=='/')
                        if(*face!=' ') // space means proceed to next index
                     {
                        face=TextValue(face, cval); if(cval.type)vtx.nrm=cval.asInt()-1; // can be invalid, because it's checked later
                     }
                  }
               }else break;
            }
            if(fvtx.elms()==3)
            {
               FTri &face=group.tri.New();
               face.pos.set(fvtx[0].pos, fvtx[1].pos, fvtx[2].pos);
               face.nrm.set(fvtx[0].nrm, fvtx[1].nrm, fvtx[2].nrm);
               face.tex.set(fvtx[0].tex, fvtx[1].tex, fvtx[2].tex);
            }else
            if(fvtx.elms()==4)
            {
               VecI4 ind (fvtx[0].pos, fvtx[1].pos, fvtx[2].pos, fvtx[3].pos);
               Quad  quad(vpos[ind.x], vpos[ind.y], vpos[ind.z], vpos[ind.w]);
               if(!quad.convex())goto as_poly; // this can happen

               FQuad &face=group.quad.New();
               face.pos=ind;
               face.nrm.set(fvtx[0].nrm, fvtx[1].nrm, fvtx[2].nrm, fvtx[3].nrm);
               face.tex.set(fvtx[0].tex, fvtx[1].tex, fvtx[2].tex, fvtx[3].tex);
            }else
            {
            as_poly:
               Vec nrm=0; REPA(fvtx) // get average face normal
               {
                  VecI2 ind(fvtx[i].pos, fvtx[(i+1)%fvtx.elms()].pos);
                  nrm+=GetNormalEdge(vpos[ind.x], vpos[ind.y]);
               }
               for(; fvtx.elms()>=3; )
               {
                  Bool added=false; // if added a face in this step
                  REP(fvtx.elms()-2)
                  {
                     VecI ind(fvtx[i].pos, fvtx[i+1].pos, fvtx[i+2].pos);
                     Tri  tri(vpos[ind.x], vpos[ind.y], vpos[ind.z]);
                     if(Dot(nrm, tri.n)>0) // if normal of this triangle is correct
                     {
                        Vec cross[3]={Cross(tri.n, tri.p[0]-tri.p[1]), Cross(tri.n, tri.p[1]-tri.p[2]), Cross(tri.n, tri.p[2]-tri.p[0])};
                        REPAD(t, fvtx)if(t<i || t>i+2)if(Cuts(vpos[fvtx[t].pos], tri, cross))goto cuts; // if any other vertex intersects with this triangle, then continue

                        {
                           FTri &face=group.tri.New();
                           face.pos=ind;
                           face.nrm.set(fvtx[i].nrm, fvtx[i+1].nrm, fvtx[i+2].nrm);
                           face.tex.set(fvtx[i].tex, fvtx[i+1].tex, fvtx[i+2].tex);
                           fvtx.remove(i+1, true);
                           added=true;
                        }
                     cuts:;
                     }
                  }
                  if(!added) // add all remaining
                  {
                     REP(fvtx.elms()-2)
                     {
                        FTri &face=group.tri.New();
                        face.pos.set(fvtx[i].pos, fvtx[i+1].pos, fvtx[i+2].pos);
                        face.nrm.set(fvtx[i].nrm, fvtx[i+1].nrm, fvtx[i+2].nrm);
                        face.tex.set(fvtx[i].tex, fvtx[i+1].tex, fvtx[i+2].tex);
                        fvtx.remove(i+1, true);
                     }
                     break;
                  }
               }
            }
         invalid:
            fvtx.clear();
         }else
         if(c=='#') // comment
         {
            f.getLine(line);
            if(Contains(line, "Luxology modo"))flip_normal_y=true;
         }else
         if(c!=0xA)f.skipLine();
      }

      if(flip_normal_y)REPAO(materials).flip_normal_y=true;

      REPA(groups)if(!groups[i].faces())groups.remove(i, true); // remove empty groups

      // create mesh
      if(mesh)
      {
         mesh->create(groups.elms());
         FREPA(groups)
         {
            Group    &cur =groups[i];
            MeshPart &part=mesh->parts[i]; Set(part.name, cur.name);
            MeshBase &mshb=part.base;
            mshb.create(cur.tri.elms()*3+cur.quad.elms()*4, 0, cur.tri.elms(), cur.quad.elms(), (vnrm.elms() ? VTX_NRM : 0)|(vtex.elms() ? VTX_TEX0 : 0));
            Vec   *pos =mshb.vtx .pos ();
            Vec   *nrm =mshb.vtx .nrm ();
            Vec2  *tex =mshb.vtx .tex0();
            VecI  *tri =mshb.tri .ind ();
            VecI4 *quad=mshb.quad.ind ();
            Int    vtxs=0;
            FREPA(cur.tri)
            {
               FTri &ftri=cur.tri[i];
               if(pos)
               {
                  pos[0]=vpos[ftri.pos.c[0]];
                  pos[1]=vpos[ftri.pos.c[1]];
                  pos[2]=vpos[ftri.pos.c[2]]; pos+=3;
               }
               if(nrm)
               {
                  nrm[0]=(InRange(ftri.nrm.c[0], vnrm) ? vnrm[ftri.nrm.c[0]] : VecZero);
                  nrm[1]=(InRange(ftri.nrm.c[1], vnrm) ? vnrm[ftri.nrm.c[1]] : VecZero);
                  nrm[2]=(InRange(ftri.nrm.c[2], vnrm) ? vnrm[ftri.nrm.c[2]] : VecZero); nrm+=3;
               }
               if(tex)
               {
                  tex[0]=(InRange(ftri.tex.c[0], vtex) ? vtex[ftri.tex.c[0]] : 0);
                  tex[1]=(InRange(ftri.tex.c[1], vtex) ? vtex[ftri.tex.c[1]] : 0);
                  tex[2]=(InRange(ftri.tex.c[2], vtex) ? vtex[ftri.tex.c[2]] : 0); tex+=3;
               }
               (tri++)->set(vtxs, vtxs+1, vtxs+2); vtxs+=3;
            }
            FREPA(cur.quad)
            {
               FQuad &fquad=cur.quad[i];
               if(pos)
               {
                  pos[0]=vpos[fquad.pos.c[0]];
                  pos[1]=vpos[fquad.pos.c[1]];
                  pos[2]=vpos[fquad.pos.c[2]];
                  pos[3]=vpos[fquad.pos.c[3]]; pos+=4;
               }
               if(nrm)
               {
                  nrm[0]=(InRange(fquad.nrm.c[0], vnrm) ? vnrm[fquad.nrm.c[0]] : VecZero);
                  nrm[1]=(InRange(fquad.nrm.c[1], vnrm) ? vnrm[fquad.nrm.c[1]] : VecZero);
                  nrm[2]=(InRange(fquad.nrm.c[2], vnrm) ? vnrm[fquad.nrm.c[2]] : VecZero);
                  nrm[3]=(InRange(fquad.nrm.c[3], vnrm) ? vnrm[fquad.nrm.c[3]] : VecZero); nrm+=4;
               }
               if(tex)
               {
                  tex[0]=(InRange(fquad.tex.c[0], vtex) ? vtex[fquad.tex.c[0]] : 0);
                  tex[1]=(InRange(fquad.tex.c[1], vtex) ? vtex[fquad.tex.c[1]] : 0);
                  tex[2]=(InRange(fquad.tex.c[2], vtex) ? vtex[fquad.tex.c[2]] : 0);
                  tex[3]=(InRange(fquad.tex.c[3], vtex) ? vtex[fquad.tex.c[3]] : 0); tex+=4;
               }
               (quad++)->set(vtxs, vtxs+1, vtxs+2, vtxs+3); vtxs+=4;
            }
            mshb.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down
         }
         mesh->mirrorX().setBox();
         REPA(*mesh)if(!mesh->parts[i].base.vtx.nrm())mesh->parts[i].base.setNormals();
         CleanMesh(*mesh);
      }

      // material indexes
      if(part_material_index)FREPA(groups)part_material_index.add(groups[i].material);
      return true;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
