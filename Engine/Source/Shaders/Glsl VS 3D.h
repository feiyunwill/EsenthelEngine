#include "Glsl 3D.h"
#define MAX_MATRIX 60

PAR HP Vec2    GrassRangeMulAdd; // must be HP
PAR HP Vec2    SkyFracMulAdd; // better do HP because we can operate on large view ranges
PAR HP Vec2    VertexFogMulAdd; // must be HP
PAR LP Vec     VertexFogColor;
PAR MP Vec4    FogColor_Density;
PAR HP Vec4    ClipPlane;
//PAR  Matrix  ViewMatrix[MAX_MATRIX]; Adreno 220 Android 2.3 complains about insufficient vertex uniform vectors, this also causes bug in Windows Nvidia GeForce GTX 460 where full array of matrixes is not always detected as used in the shader when skinning is present
PAR HP Vec4    ViewMatrix[MAX_MATRIX*3];
PAR MP Vec     ObjVel[MAX_MATRIX];
PAR MP Vec     ObjAngVel;
PAR MP Vec     CamAngVel;
PAR HP Matrix4(ProjMatrix);

MP Vec ViewMatrixY  () {MP Int i=gl_InstanceID*3; return Vec(ViewMatrix[i].y, ViewMatrix[i+1].y, ViewMatrix[i+2].y);}
HP Vec ViewMatrixPos() {MP Int i=gl_InstanceID*3; return Vec(ViewMatrix[i].w, ViewMatrix[i+1].w, ViewMatrix[i+2].w);}

LP Vec FogColor  () {return FogColor_Density.rgb;}
MP Flt FogDensity() {return FogColor_Density.a  ;}

/*#if 0
HP Vec TransformPos(HP Vec v) {return Transform (v, ViewMatrix[0]);}
MP Vec TransformDir(MP Vec v) {return Transform3(v, ViewMatrix[0]);}

HP Vec TransformPos(HP Vec v, MP VecI bone, HP Vec weight) {return weight.x*Transform (v, ViewMatrix[bone.x]) + weight.y*Transform (v, ViewMatrix[bone.y]) + weight.z*Transform (v, ViewMatrix[bone.z]);}
MP Vec TransformDir(MP Vec v, MP VecI bone, MP Vec weight) {return weight.x*Transform3(v, ViewMatrix[bone.x]) + weight.y*Transform3(v, ViewMatrix[bone.y]) + weight.z*Transform3(v, ViewMatrix[bone.z]);}
#else*/
HP Vec TransformPos(HP Vec v) // main matrix
{
   return Vec(Dot(v, ViewMatrix[0].xyz) + ViewMatrix[0].w,
              Dot(v, ViewMatrix[1].xyz) + ViewMatrix[1].w,
              Dot(v, ViewMatrix[2].xyz) + ViewMatrix[2].w);
}
MP Vec TransformDir(MP Vec v) // main matrix
{
   MP Vec m0=ViewMatrix[0].xyz, m1=ViewMatrix[1].xyz, m2=ViewMatrix[2].xyz;
   return Vec(Dot(v, m0),
              Dot(v, m1),
              Dot(v, m2));
}

HP Vec TransformPos(HP Vec v, MP Int i) // i-th matrix
{
   i*=3;
   return Vec(Dot(v, ViewMatrix[i  ].xyz) + ViewMatrix[i  ].w,
              Dot(v, ViewMatrix[i+1].xyz) + ViewMatrix[i+1].w,
              Dot(v, ViewMatrix[i+2].xyz) + ViewMatrix[i+2].w);
}
MP Vec TransformDir(MP Vec v, MP Int i) // i-th matrix
{
   i*=3;
   MP Vec m0=ViewMatrix[i].xyz, m1=ViewMatrix[i+1].xyz, m2=ViewMatrix[i+2].xyz;
   return Vec(Dot(v, m0),
              Dot(v, m1),
              Dot(v, m2));
}
#ifdef GL_ES // GLSL may not support "#if GL_ES" if GL_ES is not defined
   HP Vec TransformPos(HP Vec v, MP VecI bone, HP Vec weight) {return weight.x*TransformPos(v, bone.x) + weight.y*TransformPos(v, bone.y);} // use only 2 weights on OpenGL ES
   MP Vec TransformDir(MP Vec v, MP VecI bone, MP Vec weight) {return weight.x*TransformDir(v, bone.x) + weight.y*TransformDir(v, bone.y);} // use only 2 weights on OpenGL ES
   MP Vec GetBoneVel  (          MP VecI bone, MP Vec weight) {return weight.x*      ObjVel[   bone.x] + weight.y*      ObjVel[   bone.y];} // use only 2 weights on OpenGL ES
#else
   HP Vec TransformPos(HP Vec v, MP VecI bone, HP Vec weight) {return weight.x*TransformPos(v, bone.x) + weight.y*TransformPos(v, bone.y) + weight.z*TransformPos(v, bone.z);}
   MP Vec TransformDir(MP Vec v, MP VecI bone, MP Vec weight) {return weight.x*TransformDir(v, bone.x) + weight.y*TransformDir(v, bone.y) + weight.z*TransformDir(v, bone.z);}
   MP Vec GetBoneVel  (          MP VecI bone, MP Vec weight) {return weight.x*      ObjVel[   bone.x] + weight.y*      ObjVel[   bone.y] + weight.z*      ObjVel[   bone.z];}
#endif
//#endif
HP Vec4 Project(HP Vec v) {return Transform(v, ProjMatrix);}

HP Vec ObjWorldPos() {return Transform(ViewMatrixPos(), CamMatrix);} // MatrixPos(ViewMatrix[])

PAR MP Vec4 BendFactor;

#define GrassBendFreq  1.0
#define GrassBendScale 0.18

#define LeafBendFreq   2.0
#define LeafBendScale  0.13
#define LeafsBendScale (LeafBendScale/2.0)

MP Vec2 GetGrassBend(MP Vec center)
{
   MP Flt offset=Sum(center.xz*(Vec2(0.7, 0.9)*GrassBendFreq));
   return Vec2((0.28*GrassBendScale)*Sin(offset+BendFactor.x) + (0.32*GrassBendScale)*Sin(offset+BendFactor.y),
               (0.18*GrassBendScale)*Sin(offset+BendFactor.z) + (0.24*GrassBendScale)*Sin(offset+BendFactor.w));
}
MP Vec2 GetLeafBend(MP Vec center)
{
   MP Flt offset=Sum(center.xy*(Vec2(0.7, 0.8)*LeafBendFreq));
   return Vec2((0.28*LeafBendScale)*Sin(offset+BendFactor.x) + (0.32*LeafBendScale)*Sin(offset+BendFactor.y),
               (0.18*LeafBendScale)*Sin(offset+BendFactor.z) + (0.24*LeafBendScale)*Sin(offset+BendFactor.w));
}
MP Vec2 GetLeafsBend(MP Vec center)
{
   MP Flt offset=Sum(center.xy*(Vec2(0.7, 0.8)*LeafBendFreq));
   return Vec2((0.28*LeafsBendScale)*Sin(offset+BendFactor.x) + (0.32*LeafsBendScale)*Sin(offset+BendFactor.y),
               (0.18*LeafsBendScale)*Sin(offset+BendFactor.z) + (0.24*LeafsBendScale)*Sin(offset+BendFactor.w));
}
MP Flt GrassFadeOut()
{
   return Sat(Length2(ViewMatrixPos())*GrassRangeMulAdd.x+GrassRangeMulAdd.y); // - fade_out // MatrixPos(ViewMatrix[])
}
MP Vec BendGrass(MP Vec local_pos)
{
   HP Vec  world_pos=ObjWorldPos();
   MP Flt  b        =Cube(Sat(local_pos.y));
   MP Vec2 bend     =GetGrassBend(world_pos)*(b*Length(ViewMatrixY())); // MatrixY(ViewMatrix[])

   MP Vec mc0=CamMatrix[0].xyz, mc1=CamMatrix[1].xyz;
   return mc0*bend.x
         +mc1*bend.y;
}
HP Vec BendLeaf(MP Vec center, HP Vec pos)
{
   pos-=center;
   MP Vec2 bend=GetLeafBend(center);
   pos.xy=Rotate(pos.xy, Vec2(Cos(bend.x), Sin(bend.x)));
   pos.zy=Rotate(pos.zy, Vec2(Cos(bend.y), Sin(bend.y)));
   pos+=center;
   return pos;
}
HP Vec BendLeafs(MP Vec center, MP Flt offset, HP Vec pos)
{
   pos-=center;
   MP Vec2 bend=GetLeafsBend(center+offset);
   pos.xy=Rotate(pos.xy, Vec2(Cos(bend.x), Sin(bend.x)));
   pos.zy=Rotate(pos.zy, Vec2(Cos(bend.y), Sin(bend.y)));
   pos+=center;
   return pos;
}
void UpdateVelocities_VS(inout MP Vec vel, HP Vec local_pos, HP Vec view_space_pos)
{
   vel-=TransformDir(Cross(local_pos      , ObjAngVel), gl_InstanceID);
   vel+=             Cross( view_space_pos, CamAngVel);
}
