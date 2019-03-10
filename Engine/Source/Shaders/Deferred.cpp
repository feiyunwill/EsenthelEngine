/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
#define MacroFrom   192.0f
#define MacroTo     320.0f
#define MacroMax    0.70f
#define MacroScale (1.0f/32)

#define DEFAULT_TEX_SIZE 1024.0f // 1024x1024

#define PARALLAX_MODE 1 // 1=best

#define RELIEF_STEPS_MAX    32
#define RELIEF_STEPS_BINARY 3 // 3 works good in most cases, 4 could be used for absolute best quality
#define RELIEF_STEPS_MUL    0.75f // 0.75f gets slightly worse quality but better performance, 1.0f gets full quality but slower performance, default=0.75f
#define RELIEF_LOD_OFFSET   0.33f // values >0 increase performance (by using fewer steps and smaller LOD's) which also makes results more soft and flat helping to reduce jitter for distant pixels, default=0.33f
#define RELIEF_TAN_POS      1 // 0=gets worse quality but better performance (not good for triangles with vertexes with very different normals or for surfaces very close to camera), 1=gets full quality but slower performance, default=1
#define RELIEF_DEC_NRM      1 // if reduce relief bump intensity where there are big differences between vtx normals, tangents and binormals, default=1
#define RELIEF_MODE         1 // 1=best
#define RELIEF_Z_LIMIT      0.4f // smaller values may cause leaking (UV swimming), and higher reduce bump intensity at angles, default=0.4f
#define RELIEF_LOD_TEST     0 // close to camera (test enabled=4.76 fps, test disabled=4.99 fps), far from camera (test enabled=9.83 fps, test disabled=9.52 fps), conclusion: this test reduces performance when close to camera by a similar factor to when far away, however since more likely pixels will be close to camera (as for distant LOD's other shaders are used) we prioritize close to camera performance, so this check should be disabled, default=0
/******************************************************************************/
#define PARAMS             \
   uniform Bool skin      ,\
   uniform Int  materials ,\
   uniform Int  textures  ,\
   uniform Int  bump_mode ,\
   uniform Bool alpha_test,\
   uniform Bool light_map ,\
   uniform Bool detail    ,\
   uniform Bool macro     ,\
   uniform Bool rflct     ,\
   uniform Bool color     ,\
   uniform Bool mtrl_blend,\
   uniform Bool heightmap ,\
   uniform Int  fx        ,\
   uniform Bool tesselate      
/******************************************************************************/
struct VS_PS
{
   Vec2    tex     :TEXCOORD0;
   Vec     pos     :TEXCOORD1;
   Vec    _tpos    :TEXCOORD2;
   Vec     vel     :TEXCOORD3;
   Matrix3 mtrx    :TEXCOORD4; // !! may not be Normalized !!
   Vec2    tex_l   :TEXCOORD7;
   VecH4   material:COLOR0   ;
   VecH    col     :COLOR1   ;
#if   DX9
   Half    fade_out:COLOR0   ; // can't use TEXCOORD8 on DX9
#elif GL
   #define fade_out material.x // can't use TEXCOORD8 and can't reuse COLOR0 on GL
#else
   Half    fade_out:TEXCOORD8;
#endif

   Vec tpos() {return Normalize(_tpos);}
};
/******************************************************************************/
// VS
/******************************************************************************/
void VS
(
   VtxInput vtx,

   out VS_PS O,
   out Vec4  O_vtx:POSITION,

   IF_IS_CLIP

   PARAMS
)
{
   Vec  pos=vtx.pos();
   VecH nrm, tan; if(bump_mode>=SBUMP_FLAT)nrm=vtx.nrm(); if(bump_mode>SBUMP_FLAT)tan=vtx.tan(nrm, heightmap);

   if(textures || detail)O.tex     =vtx.tex     (heightmap);
   if(light_map         )O.tex_l   =vtx.tex1    ();
   if(materials>1       )O.material=vtx.material();

   if(materials<=1)O.col.rgb=MaterialColor3();/*else
   if(!mtrl_blend )
   {
                      O.col.rgb =O.material.x*MultiMaterial0Color3()
                                +O.material.y*MultiMaterial1Color3();
      if(materials>=3)O.col.rgb+=O.material.z*MultiMaterial2Color3();
      if(materials>=4)O.col.rgb+=O.material.w*MultiMaterial3Color3();
   }*/
   if(color)
   {
      if(materials<=1 /*|| !mtrl_blend*/)O.col.rgb*=vtx.color3();
      else                               O.col.rgb =vtx.color3();
   }

   if(heightmap && textures && materials==1)O.tex*=MaterialTexScale();

   if(fx==FX_LEAF)
   {
      if(bump_mode> SBUMP_FLAT)BendLeaf(vtx.hlp(), pos, nrm, tan);else
      if(bump_mode==SBUMP_FLAT)BendLeaf(vtx.hlp(), pos, nrm     );else
                               BendLeaf(vtx.hlp(), pos          );
   }
   if(fx==FX_LEAFS)
   {
      if(bump_mode> SBUMP_FLAT)BendLeafs(vtx.hlp(), vtx.size(), pos, nrm, tan);else
      if(bump_mode==SBUMP_FLAT)BendLeafs(vtx.hlp(), vtx.size(), pos, nrm     );else
                               BendLeafs(vtx.hlp(), vtx.size(), pos          );
   }

   if(!skin)
   {
   #if MODEL>=SM_4 || MODEL==SM_GL
      if(true) // instance
      {
         O.vel=ObjVel[vtx.instance()]; // #PER_INSTANCE_VEL
         O.pos=TransformPos(pos, vtx.instance());

         if(bump_mode>=SBUMP_FLAT)O.mtrx[2]=TransformDir(nrm, vtx.instance());
         if(bump_mode> SBUMP_FLAT)O.mtrx[0]=TransformDir(tan, vtx.instance());
         if(fx==FX_GRASS)
         {
              BendGrass(pos, O.pos, vtx.instance());
            O.fade_out=GrassFadeOut(vtx.instance());
         }
         UpdateVelocities_VS(O.vel, pos, O.pos, vtx.instance());
      }else
   #endif
      {
         O.vel=ObjVel[0];
         O.pos=TransformPos(pos);

         if(bump_mode>=SBUMP_FLAT)O.mtrx[2]=TransformDir(nrm);
         if(bump_mode> SBUMP_FLAT)O.mtrx[0]=TransformDir(tan);
         if(fx==FX_GRASS)
         {
            BendGrass(pos, O.pos);
            O.fade_out=GrassFadeOut();
         }
         UpdateVelocities_VS(O.vel, pos, O.pos);
      }
   }else
   {
      VecI bone=vtx.bone();
      O.vel=GetBoneVel  (     bone, vtx.weight());
      O.pos=TransformPos(pos, bone, vtx.weight());

      if(bump_mode>=SBUMP_FLAT)O.mtrx[2]=TransformDir(nrm, bone, vtx.weight());
      if(bump_mode> SBUMP_FLAT)O.mtrx[0]=TransformDir(tan, bone, vtx.weight());
      UpdateVelocities_VS(O.vel, pos, O.pos);
   }

   // normalize (have to do all at the same time, so all have the same lengths)
   if(bump_mode> SBUMP_FLAT // calculating binormal (this also covers the case when we have tangent from heightmap which is not Normalized)
   || bump_mode==SBUMP_RELIEF && RELIEF_DEC_NRM // needed for RELIEF_DEC_NRM effect
   || tesselate) // needed for tesselation
   {
                              O.mtrx[2]=Normalize(O.mtrx[2]);
      if(bump_mode>SBUMP_FLAT)O.mtrx[0]=Normalize(O.mtrx[0]);
   }

   if(bump_mode>SBUMP_FLAT)O.mtrx[1]=vtx.bin(O.mtrx[2], O.mtrx[0], heightmap);
   if((bump_mode>=SBUMP_PARALLAX_MIN && bump_mode<=SBUMP_PARALLAX_MAX)
   || (bump_mode==SBUMP_RELIEF       && !RELIEF_TAN_POS              ))O._tpos=mul(O.mtrx, -O.pos);

   O_vtx=Project(O.pos); CLIP(O.pos);
}
/******************************************************************************/
// PS
/******************************************************************************/
void PS
(
   VS_PS I,

   IF_IS_FRONT

   out DeferredSolidOutput output,

   PARAMS
)
{
   VecH nrm;
   Half glow, specular, sss;

   if(bump_mode==SBUMP_ZERO)
   {
      glow    =MaterialGlow();
      specular=MaterialSpecular();
      sss     =MaterialSss();
      nrm     =0;
   }else
   if(materials==1)
   {
      // apply tex coord bump offset
      if(textures==2)
      {
         if(bump_mode>=SBUMP_PARALLAX_MIN && bump_mode<=SBUMP_PARALLAX_MAX) // parallax mapping
         {
            const Int steps=bump_mode-SBUMP_PARALLAX0;

            Vec tpos=I.tpos();
         #if   PARALLAX_MODE==0 // too flat
            Flt scale=MaterialBump()/steps;
         #elif PARALLAX_MODE==1 // best results (not as flat, and not much aliasing)
            Flt scale=MaterialBump()/(steps*Lerp(1, tpos.z, tpos.z)); // MaterialBump()/steps/Lerp(1, tpos.z, tpos.z);
         #elif PARALLAX_MODE==2 // generates too steep walls (not good for parallax)
            Flt scale=MaterialBump()/(steps*Lerp(1, tpos.z, Sat(tpos.z/0.5f))); // MaterialBump()/steps/Lerp(1, tpos.z, tpos.z);
         #elif PARALLAX_MODE==3 // introduces a bit too much aliasing/artifacts on surfaces perpendicular to view direction
            Flt scale=MaterialBump()/steps*(2-tpos.z); // MaterialBump()/steps*Lerp(1, 1/tpos.z, tpos.z)
         #else // correct however introduces way too much aliasing/artifacts on surfaces perpendicular to view direction
            Flt scale=MaterialBump()/steps/tpos.z;
         #endif
            tpos.xy*=scale;
            UNROLL for(Int i=0; i<steps; i++)I.tex+=(Tex(Col, I.tex).w-0.5f)*tpos.xy;
         }else
         if(bump_mode==SBUMP_RELIEF) // relief mapping
         {
         #if RELIEF_LOD_TEST
            BRANCH if(GetLod(I.tex, DEFAULT_TEX_SIZE)<=4)
         #endif
            {
            #if MODEL>=SM_4
               Flt TexSize, TexHeight; Col.GetDimensions(TexSize, TexHeight);
            #else
               Flt TexSize=DEFAULT_TEX_SIZE;
            #endif
               Flt lod=Max(0, GetLod(I.tex, TexSize)+RELIEF_LOD_OFFSET); // yes, can be negative, so use Max(0) to avoid increasing number of steps when surface is close to camera
             //lod=Trunc(lod); don't do this as it would reduce performance and generate more artifacts, with this disabled, we generate fewer steps gradually, and blend with the next MIP level softening results

            #if RELIEF_TAN_POS
               Vec tpos=Normalize(mul(I.mtrx, -I.pos));
            #else
               Vec tpos=I.tpos();
            #endif

            #if   RELIEF_MODE==0
               Flt scale=MaterialBump();
            #elif RELIEF_MODE==1 // best
               Flt scale=MaterialBump()/Lerp(1, tpos.z, Sat(tpos.z/RELIEF_Z_LIMIT));
            #elif RELIEF_MODE==2 // produces slight aliasing/artifacts on surfaces perpendicular to view direction
               Flt scale=MaterialBump()/Max(tpos.z, RELIEF_Z_LIMIT);
            #else // correct however introduces way too much aliasing/artifacts on surfaces perpendicular to view direction
               Flt scale=MaterialBump()/tpos.z;
            #endif

            #if RELIEF_DEC_NRM
               scale*=Length2(I.mtrx[0])*Length2(I.mtrx[1])*Length2(I.mtrx[2]); // vtx matrix vectors are interpolated linearly, which means that if there are big differences between vtx vectors, then their length will be smaller and smaller, for example if #0 vtx normal is (1,0), and #1 vtx normal is (0,1), then interpolated value between them will be (0.5, 0.5)
            #endif
               tpos.xy*=-scale;

               Flt length=Length(tpos.xy) * TexSize / Pow(2, lod);
               if(RELIEF_STEPS_MUL!=1)if(lod>0)length*=RELIEF_STEPS_MUL; // don't use this for first LOD

               I.tex-=tpos.xy*0.5f;

               Int  steps   =Mid(length, 0, RELIEF_STEPS_MAX);
               Flt  stp     =1.0f/(steps+1),
                    ray     =1;
               Vec2 tex_step=tpos.xy*stp;

            #if 1 // linear + interval search (faster)
               // linear search
               Flt height_next, height_prev=0.5f; // use 0.5 as approximate average value, we could do "TexLodI(Col, I.tex, lod).w", however in tests that wasn't needed but only reduced performance
               LOOP for(Int i=0; ; i++)
               {
                  ray  -=stp;
                  I.tex+=tex_step;
                  height_next=TexLodI(Col, I.tex, lod).w;
                  if(i>=steps || height_next>=ray)break;
                  height_prev=height_next;
               }

               // interval search
               if(1)
               {
                  Flt ray_prev=ray+stp;
                  // prev pos: I.tex-tex_step, height_prev-ray_prev
                  // next pos: I.tex         , height_next-ray
                  Flt hn=height_next-ray, hp=height_prev-ray_prev,
                      frac=Sat(hn/(hn-hp));
                  I.tex-=tex_step*frac;

                  BRANCH if(lod<=0) // extra step (needed only for closeup)
                  {
                     Flt ray_cur=ray+stp*frac,
                         height_cur=TexLodI(Col, I.tex, lod).w;
                     if( height_cur>=ray_cur) // if still below, then have to go back more, lerp between this position and prev pos
                     {
                        // prev pos: I.tex-tex_step (BUT I.tex before adjustment), height_prev-ray_prev
                        // next pos: I.tex                                       , height_cur -ray_cur
                        tex_step*=1-frac; // we've just travelled "tex_step*frac", so to go to the prev point, we need what's left, "tex_step*(1-frac)"
                     }else // we went back too far, go forward, lerp between next pos and this position
                     {
                        // prev pos: I.tex                              , height_cur -ray_cur
                        // next pos: I.tex (BUT I.tex before adjustment), height_next-ray
                        hp=hn;
                        tex_step*=-frac; // we've just travelled "tex_step*frac", so to go to the next point, we need the same way but other direction, "tex_step*-frac"
                     }
                     hn=height_cur-ray_cur;
                     frac=Sat(hn/(hn-hp));
                     I.tex-=tex_step*frac;
                  }
               }
            #else // linear + binary search (slower because requires 3 tex reads in binary to get the same results as with only 0-1 tex reads in interval)
               // linear search
               LOOP for(Int i=0; ; i++)
               {
                  ray  -=stp;
                  I.tex+=tex_step;
                  if(i>=steps || TexLodI(Col, I.tex, lod).w>=ray)break;
               }

               // binary search
               {
                  Flt ray_prev=ray+stp,
                      l=0, r=1, m=0.5f;
                  UNROLL for(Int i=0; i<RELIEF_STEPS_BINARY; i++)
                  {
                     Flt height=TexLodI(Col, I.tex-tex_step*m, lod).w;
                     if( height>Lerp(ray, ray_prev, m))l=m;else r=m;
                     m=Avg(l, r);
                  }
                  I.tex-=tex_step*m;
               }
            #endif
            }
         }
      }

      VecH4 tex_nrm; // #MaterialTextureChannelOrder
      if(textures==0)
      {
         if(detail)I.col.rgb+=GetDetail(I.tex).z;
         nrm     =Normalize(I.mtrx[2]);
         glow    =MaterialGlow    ();
         specular=MaterialSpecular();
      }else
      if(textures==1)
      {
         VecH4  tex=Tex(Col, I.tex); if(alpha_test)AlphaTest(tex.w, I.fade_out, fx);
         I.col.rgb*=tex.rgb        ; if(detail    )I.col.rgb+=GetDetail(I.tex).z;
         nrm       =Normalize(I.mtrx[2]);
         glow      =MaterialGlow    ();
         specular  =MaterialSpecular();
      }else
      if(textures==2)
      {
         tex_nrm =Tex(Nrm, I.tex);    if( alpha_test)AlphaTest(tex_nrm.a, I.fade_out, fx);
         glow    =MaterialGlow    (); if(!alpha_test)glow*=tex_nrm.a;
         specular=MaterialSpecular()*tex_nrm.z;
         if(bump_mode==SBUMP_FLAT)
         {
            nrm       =Normalize(I.mtrx[2]);
            I.col.rgb*=Tex(Col, I.tex).rgb; if(detail)I.col.rgb+=GetDetail(I.tex).z;
         }else // normal mapping
         {
            VecH      det;
            if(detail)det=GetDetail(I.tex);

                      nrm.xy =(tex_nrm.xy*2-1)*MaterialRough();
            if(detail)nrm.xy+=det.xy;
                      nrm.z  =CalcZ(nrm.xy);
                      nrm    =Normalize(Transform(nrm, I.mtrx));

                      I.col.rgb*=Tex(Col, I.tex).rgb;
            if(detail)I.col.rgb+=det.z;
         }
      }

      if(macro    )I.col.rgb =Lerp(I.col.rgb, Tex(Mac, I.tex*MacroScale).rgb, LerpRS(MacroFrom, MacroTo, I.pos.z)*MacroMax);
      if(light_map)I.col.rgb*=Tex(Lum, I.tex_l).rgb;
      sss=MaterialSss();

      // reflection
      if(rflct)
      {
         Vec rfl=Transform3(reflect(I.pos, nrm), CamMatrix);
         I.col.rgb+=TexCube(Rfl, rfl).rgb*((textures==2) ? MaterialReflect()*tex_nrm.z : MaterialReflect());
      }
   }else // materials>1
   {
      // assuming that in multi materials textures!=0
      Vec2 tex0, tex1, tex2, tex3;
                      tex0=I.tex*MultiMaterial0TexScale();
                      tex1=I.tex*MultiMaterial1TexScale();
      if(materials>=3)tex2=I.tex*MultiMaterial2TexScale();
      if(materials>=4)tex3=I.tex*MultiMaterial3TexScale();

      // apply tex coord bump offset
      if(textures==2)
      {
         if(bump_mode>=SBUMP_PARALLAX_MIN && bump_mode<=SBUMP_PARALLAX_MAX) // parallax mapping
         {
            const Int steps=bump_mode-SBUMP_PARALLAX0;

            Vec tpos=I.tpos();
         #if   PARALLAX_MODE==0 // too flat
            Flt scale=1/steps;
         #elif PARALLAX_MODE==1 // best results (not as flat, and not much aliasing)
            Flt scale=1/(steps*Lerp(1, tpos.z, tpos.z)); // 1/steps/Lerp(1, tpos.z, tpos.z);
         #elif PARALLAX_MODE==2 // generates too steep walls (not good for parallax)
            Flt scale=1/(steps*Lerp(1, tpos.z, Sat(tpos.z/0.5f)));
         #elif PARALLAX_MODE==3 // introduces a bit too much aliasing/artifacts on surfaces perpendicular to view direction
            Flt scale=1/steps*(2-tpos.z); // 1/steps*Lerp(1, 1/tpos.z, tpos.z)
         #else // correct however introduces way too much aliasing/artifacts on surfaces perpendicular to view direction
            Flt scale=1/steps/tpos.z;
         #endif
            tpos.xy*=scale;

            // (x-0.5)*bump_mul = x*bump_mul - 0.5*bump_mul
            VecH4 bump_mul; bump_mul.x=MultiMaterial0Bump();
            VecH4 bump_add; bump_mul.y=MultiMaterial1Bump(); if(materials==2){bump_mul.xy  *=I.material.xy  ; bump_add.xy  =bump_mul.xy  *-0.5f;}
            if(materials>=3)bump_mul.z=MultiMaterial2Bump(); if(materials==3){bump_mul.xyz *=I.material.xyz ; bump_add.xyz =bump_mul.xyz *-0.5f;}
            if(materials>=4)bump_mul.w=MultiMaterial3Bump(); if(materials==4){bump_mul.xyzw*=I.material.xyzw; bump_add.xyzw=bump_mul.xyzw*-0.5f;}

            UNROLL for(Int i=0; i<steps; i++) // I.tex+=(Tex(Col, I.tex).w-0.5f)*tpos.xy;
            {
                          Half h =Tex(Col , tex0).w*bump_mul.x+bump_add.x
                                 +Tex(Col1, tex1).w*bump_mul.y+bump_add.y;
               if(materials>=3)h+=Tex(Col2, tex2).w*bump_mul.z+bump_add.z;
               if(materials>=4)h+=Tex(Col3, tex3).w*bump_mul.w+bump_add.w;

               Vec2 offset=h*tpos.xy;

                               tex0+=offset;
                               tex1+=offset;
               if(materials>=3)tex2+=offset;
               if(materials>=4)tex3+=offset;
            }
         }else
         if(bump_mode==SBUMP_RELIEF)
         {
         #if RELIEF_LOD_TEST
            BRANCH if(GetLod(I.tex, DEFAULT_TEX_SIZE)<=4)
         #endif
            {
               VecH4 bump_mul; bump_mul.x=MultiMaterial0Bump(); Half avg_bump;
                               bump_mul.y=MultiMaterial1Bump(); if(materials==2){bump_mul.xy  *=I.material.xy  ; avg_bump=Sum(bump_mul.xy  );} // use 'Sum' because they're premultipled by 'I.material'
               if(materials>=3)bump_mul.z=MultiMaterial2Bump(); if(materials==3){bump_mul.xyz *=I.material.xyz ; avg_bump=Sum(bump_mul.xyz );}
               if(materials>=4)bump_mul.w=MultiMaterial3Bump(); if(materials==4){bump_mul.xyzw*=I.material.xyzw; avg_bump=Sum(bump_mul.xyzw);}

               Flt TexSize=DEFAULT_TEX_SIZE, // here we have 2..4 textures, so use a default value
                   lod=Max(0, GetLod(I.tex, TexSize)+RELIEF_LOD_OFFSET); // yes, can be negative, so use Max(0) to avoid increasing number of steps when surface is close to camera
             //lod=Trunc(lod); don't do this as it would reduce performance and generate more artifacts, with this disabled, we generate fewer steps gradually, and blend with the next MIP level softening results

            #if RELIEF_TAN_POS
               Vec tpos=Normalize(mul(I.mtrx, -I.pos));
            #else
               Vec tpos=I.tpos();
            #endif

            #if   RELIEF_MODE==0
               Flt scale=avg_bump;
            #elif RELIEF_MODE==1 // best
               Flt scale=avg_bump/Lerp(1, tpos.z, Sat(tpos.z/RELIEF_Z_LIMIT));
            #elif RELIEF_MODE==2 // produces slight aliasing/artifacts on surfaces perpendicular to view direction
               Flt scale=avg_bump/Max(tpos.z, RELIEF_Z_LIMIT);
            #else // correct however introduces way too much aliasing/artifacts on surfaces perpendicular to view direction
               Flt scale=avg_bump/tpos.z;
            #endif

            #if RELIEF_DEC_NRM
               scale*=Length2(I.mtrx[0])*Length2(I.mtrx[1])*Length2(I.mtrx[2]); // vtx matrix vectors are interpolated linearly, which means that if there are big differences between vtx vectors, then their length will be smaller and smaller, for example if #0 vtx normal is (1,0), and #1 vtx normal is (0,1), then interpolated value between them will be (0.5, 0.5)
            #endif
               tpos.xy*=-scale;

               Flt length=Length(tpos.xy) * TexSize / Pow(2, lod);
               if(RELIEF_STEPS_MUL!=1)if(lod>0)length*=RELIEF_STEPS_MUL; // don't use this for first LOD

             //I.tex-=tpos.xy*0.5f;
               Vec2 offset=tpos.xy*0.5f;
                               tex0-=offset;
                               tex1-=offset;
               if(materials>=3)tex2-=offset;
               if(materials>=4)tex3-=offset;

               Int  steps   =Mid(length, 0, RELIEF_STEPS_MAX);
               Flt  stp     =1.0f/(steps+1),
                    ray     =1;
               Vec2 tex_step=tpos.xy*stp;

            #if 1 // linear + interval search (faster)
               // linear search
               Flt height_next, height_prev=0.5f; // use 0.5 as approximate average value, we could do "TexLodI(Col, I.tex, lod).w", however in tests that wasn't needed but only reduced performance
               LOOP for(Int i=0; ; i++)
               {
                  ray-=stp;

                //I.tex+=tex_step;
                                  tex0+=tex_step;
                                  tex1+=tex_step;
                  if(materials>=3)tex2+=tex_step;
                  if(materials>=4)tex3+=tex_step;

                //height_next=TexLodI(Col, I.tex, lod).w;
                                  height_next =TexLodI(Col , tex0, lod).w*I.material.x
                                              +TexLodI(Col1, tex1, lod).w*I.material.y;
                  if(materials>=3)height_next+=TexLodI(Col2, tex2, lod).w*I.material.z;
                  if(materials>=4)height_next+=TexLodI(Col3, tex3, lod).w*I.material.w;

                  if(i>=steps || height_next>=ray)break;
                  height_prev=height_next;
               }

               // interval search
               if(1)
               {
                  Flt ray_prev=ray+stp;
                  // prev pos: I.tex-tex_step, height_prev-ray_prev
                  // next pos: I.tex         , height_next-ray
                  Flt hn=height_next-ray, hp=height_prev-ray_prev,
                      frac=Sat(hn/(hn-hp));

                //I.tex-=tex_step*frac;
                  offset=tex_step*frac;
                                  tex0-=offset;
                                  tex1-=offset;
                  if(materials>=3)tex2-=offset;
                  if(materials>=4)tex3-=offset;

                  BRANCH if(lod<=0) // extra step (needed only for closeup)
                  {
                     Flt ray_cur=ray+stp*frac,
                       //height_cur=TexLodI(Col, I.tex, lod).w;
                                     height_cur =TexLodI(Col , tex0, lod).w*I.material.x
                                                +TexLodI(Col1, tex1, lod).w*I.material.y;
                     if(materials>=3)height_cur+=TexLodI(Col2, tex2, lod).w*I.material.z;
                     if(materials>=4)height_cur+=TexLodI(Col3, tex3, lod).w*I.material.w;

                     if(height_cur>=ray_cur) // if still below, then have to go back more, lerp between this position and prev pos
                     {
                        // prev pos: I.tex-tex_step (BUT I.tex before adjustment), height_prev-ray_prev
                        // next pos: I.tex                                       , height_cur -ray_cur
                        tex_step*=1-frac; // we've just travelled "tex_step*frac", so to go to the prev point, we need what's left, "tex_step*(1-frac)"
                     }else // we went back too far, go forward, lerp between next pos and this position
                     {
                        // prev pos: I.tex                              , height_cur -ray_cur
                        // next pos: I.tex (BUT I.tex before adjustment), height_next-ray
                        hp=hn;
                        tex_step*=-frac; // we've just travelled "tex_step*frac", so to go to the next point, we need the same way but other direction, "tex_step*-frac"
                     }
                     hn=height_cur-ray_cur;
                     frac=Sat(hn/(hn-hp));

                   //I.tex-=tex_step*frac;
                     offset=tex_step*frac;
                                     tex0-=offset;
                                     tex1-=offset;
                     if(materials>=3)tex2-=offset;
                     if(materials>=4)tex3-=offset;
                  }
               }
            #else // linear + binary search (slower because requires 3 tex reads in binary to get the same results as with only 0-1 tex reads in interval)
               this needs to be updated for 4 materials

               // linear search
               LOOP for(Int i=0; ; i++)
               {
                  ray  -=stp;
                  I.tex+=tex_step;
                  if(i>=steps || TexLodI(Col, I.tex, lod).w>=ray)break;
               }

               // binary search
               {
                  Flt ray_prev=ray+stp,
                      l=0, r=1, m=0.5f;
                  UNROLL for(Int i=0; i<RELIEF_STEPS_BINARY; i++)
                  {
                     Half height=TexLodI(Col, I.tex-tex_step*m, lod).w;
                     if(  height>Lerp(ray, ray_prev, m))l=m;else r=m;
                     m=Avg(l, r);
                  }
                  I.tex-=tex_step*m;
               }
            #endif
            }
         }
      }

      // detail texture
      VecH det0, det1, det2, det3;
      if(  detail) // #MaterialTextureChannelOrder
      {
                         det0=Tex(Det , tex0*MultiMaterial0DetScale()).xyz*MultiMaterial0DetMul()+MultiMaterial0DetAdd();
                         det1=Tex(Det1, tex1*MultiMaterial1DetScale()).xyz*MultiMaterial1DetMul()+MultiMaterial1DetAdd();
         if(materials>=3)det2=Tex(Det2, tex2*MultiMaterial2DetScale()).xyz*MultiMaterial2DetMul()+MultiMaterial2DetAdd();
         if(materials>=4)det3=Tex(Det3, tex3*MultiMaterial3DetScale()).xyz*MultiMaterial3DetMul()+MultiMaterial3DetAdd();
      }

      // macro texture
      Half mac_blend;
      if(  macro)mac_blend=LerpRS(MacroFrom, MacroTo, I.pos.z)*MacroMax;

      // combine color + detail + macro !! do this first because it may modify 'I.material' which affects secondary texture !!
      VecH tex;
      if(mtrl_blend)
      {
         VecH4 col0, col1, col2, col3;
                          col0=Tex(Col , tex0); col0.rgb*=MultiMaterial0Color3(); I.material.x=MultiMaterialWeight(I.material.x, col0.a);
                          col1=Tex(Col1, tex1); col1.rgb*=MultiMaterial1Color3(); I.material.y=MultiMaterialWeight(I.material.y, col1.a); if(materials==2)I.material.xy  /=I.material.x+I.material.y;
         if(materials>=3){col2=Tex(Col2, tex2); col2.rgb*=MultiMaterial2Color3(); I.material.z=MultiMaterialWeight(I.material.z, col2.a); if(materials==3)I.material.xyz /=I.material.x+I.material.y+I.material.z;}
         if(materials>=4){col3=Tex(Col3, tex3); col3.rgb*=MultiMaterial3Color3(); I.material.w=MultiMaterialWeight(I.material.w, col3.a); if(materials==4)I.material.xyzw/=I.material.x+I.material.y+I.material.z+I.material.w;}

                         {if(detail)col0.rgb+=det0.z; if(macro)col0.rgb=Lerp(col0.rgb, Tex(Mac , tex0*MacroScale).rgb, MultiMaterial0Macro()*mac_blend); tex =I.material.x*col0.rgb;}
                         {if(detail)col1.rgb+=det1.z; if(macro)col1.rgb=Lerp(col1.rgb, Tex(Mac1, tex1*MacroScale).rgb, MultiMaterial1Macro()*mac_blend); tex+=I.material.y*col1.rgb;}
         if(materials>=3){if(detail)col2.rgb+=det2.z; if(macro)col2.rgb=Lerp(col2.rgb, Tex(Mac2, tex2*MacroScale).rgb, MultiMaterial2Macro()*mac_blend); tex+=I.material.z*col2.rgb;}
         if(materials>=4){if(detail)col3.rgb+=det3.z; if(macro)col3.rgb=Lerp(col3.rgb, Tex(Mac3, tex3*MacroScale).rgb, MultiMaterial3Macro()*mac_blend); tex+=I.material.w*col3.rgb;}
      }else
      {
                         {VecH col0=Tex(Col , tex0).rgb; if(detail)col0+=det0.z; if(macro)col0=Lerp(col0, Tex(Mac , tex0*MacroScale).rgb, MultiMaterial0Macro()*mac_blend); tex =I.material.x*col0*MultiMaterial0Color3();}
                         {VecH col1=Tex(Col1, tex1).rgb; if(detail)col1+=det1.z; if(macro)col1=Lerp(col1, Tex(Mac1, tex1*MacroScale).rgb, MultiMaterial1Macro()*mac_blend); tex+=I.material.y*col1*MultiMaterial1Color3();}
         if(materials>=3){VecH col2=Tex(Col2, tex2).rgb; if(detail)col2+=det2.z; if(macro)col2=Lerp(col2, Tex(Mac2, tex2*MacroScale).rgb, MultiMaterial2Macro()*mac_blend); tex+=I.material.z*col2*MultiMaterial2Color3();}
         if(materials>=4){VecH col3=Tex(Col3, tex3).rgb; if(detail)col3+=det3.z; if(macro)col3=Lerp(col3, Tex(Mac3, tex3*MacroScale).rgb, MultiMaterial3Macro()*mac_blend); tex+=I.material.w*col3*MultiMaterial3Color3();}
      }
      if(materials<=1 /*|| !mtrl_blend*/ || color)I.col.rgb*=tex;
      else                                        I.col.rgb =tex;

      // normal, specular, glow !! do this second after modifying 'I.material' !!
      if(textures<=1)
      {
         nrm=Normalize(I.mtrx[2]);

                   VecH2 tex =I.material.x*MultiMaterial0NormalAdd().zw // #MaterialTextureChannelOrder
                             +I.material.y*MultiMaterial1NormalAdd().zw;
         if(materials>=3)tex+=I.material.z*MultiMaterial2NormalAdd().zw;
         if(materials>=4)tex+=I.material.w*MultiMaterial3NormalAdd().zw;

         specular=tex.x;
         glow    =tex.y;

         // reflection
         if(rflct)
         {
            Vec rfl=Transform3(reflect(I.pos, nrm), CamMatrix);
                            I.col.rgb+=TexCube(Rfl , rfl).rgb*(MultiMaterial0Reflect()*I.material.x);
                            I.col.rgb+=TexCube(Rfl1, rfl).rgb*(MultiMaterial1Reflect()*I.material.y);
            if(materials>=3)I.col.rgb+=TexCube(Rfl2, rfl).rgb*(MultiMaterial2Reflect()*I.material.z);
            if(materials>=4)I.col.rgb+=TexCube(Rfl3, rfl).rgb*(MultiMaterial3Reflect()*I.material.w);
         }
      }else
      {
         Half tex_spec[4];

         if(bump_mode==SBUMP_FLAT)
         {
            nrm=Normalize(I.mtrx[2]);

                             VecH2 tex; // #MaterialTextureChannelOrder
                            {VecH2 nrm0; nrm0=Tex(Nrm , tex0).zw; if(rflct)tex_spec[0]=nrm0.x; nrm0=nrm0*MultiMaterial0NormalMul().zw+MultiMaterial0NormalAdd().zw; tex =I.material.x*nrm0;}
                            {VecH2 nrm1; nrm1=Tex(Nrm1, tex1).zw; if(rflct)tex_spec[1]=nrm1.x; nrm1=nrm1*MultiMaterial1NormalMul().zw+MultiMaterial1NormalAdd().zw; tex+=I.material.y*nrm1;}
            if(materials>=3){VecH2 nrm2; nrm2=Tex(Nrm2, tex2).zw; if(rflct)tex_spec[2]=nrm2.x; nrm2=nrm2*MultiMaterial2NormalMul().zw+MultiMaterial2NormalAdd().zw; tex+=I.material.z*nrm2;}
            if(materials>=4){VecH2 nrm3; nrm3=Tex(Nrm3, tex3).zw; if(rflct)tex_spec[3]=nrm3.x; nrm3=nrm3*MultiMaterial3NormalMul().zw+MultiMaterial3NormalAdd().zw; tex+=I.material.w*nrm3;}

            specular=tex.x;
            glow    =tex.y;
         }else // normal mapping
         {
                             VecH4 tex; // #MaterialTextureChannelOrder
                            {VecH4 nrm0=Tex(Nrm , tex0); if(rflct)tex_spec[0]=nrm0.z; nrm0=nrm0*MultiMaterial0NormalMul()+MultiMaterial0NormalAdd(); if(detail)nrm0.xy+=det0.xy; tex =I.material.x*nrm0;}
                            {VecH4 nrm1=Tex(Nrm1, tex1); if(rflct)tex_spec[1]=nrm1.z; nrm1=nrm1*MultiMaterial1NormalMul()+MultiMaterial1NormalAdd(); if(detail)nrm1.xy+=det1.xy; tex+=I.material.y*nrm1;}
            if(materials>=3){VecH4 nrm2=Tex(Nrm2, tex2); if(rflct)tex_spec[2]=nrm2.z; nrm2=nrm2*MultiMaterial2NormalMul()+MultiMaterial2NormalAdd(); if(detail)nrm2.xy+=det2.xy; tex+=I.material.z*nrm2;}
            if(materials>=4){VecH4 nrm3=Tex(Nrm3, tex3); if(rflct)tex_spec[3]=nrm3.z; nrm3=nrm3*MultiMaterial3NormalMul()+MultiMaterial3NormalAdd(); if(detail)nrm3.xy+=det3.xy; tex+=I.material.w*nrm3;}

            nrm=VecH(tex.xy, CalcZ(tex.xy));
            nrm=Normalize(Transform(nrm, I.mtrx));

            specular=tex.z;
            glow    =tex.w;
         }

         // reflection
         if(rflct)
         {
            Vec rfl=Transform3(reflect(I.pos, nrm), CamMatrix);
                            I.col.rgb+=TexCube(Rfl , rfl).rgb*(MultiMaterial0Reflect()*I.material.x*tex_spec[0]);
                            I.col.rgb+=TexCube(Rfl1, rfl).rgb*(MultiMaterial1Reflect()*I.material.y*tex_spec[1]);
            if(materials>=3)I.col.rgb+=TexCube(Rfl2, rfl).rgb*(MultiMaterial2Reflect()*I.material.z*tex_spec[2]);
            if(materials>=4)I.col.rgb+=TexCube(Rfl3, rfl).rgb*(MultiMaterial3Reflect()*I.material.w*tex_spec[3]);
         }
      }

      sss=0;
   }

   if(fx!=FX_GRASS && fx!=FX_LEAF && fx!=FX_LEAFS)BackFlip(nrm, front);

   I.col.rgb+=Highlight.rgb;
   UpdateColorBySss(I.col.rgb, nrm, sss);

   output.color   (I.col.rgb   );
   output.glow    (glow        );
   output.normal  (nrm         );
   output.specular(specular    );
   output.velocity(I.vel, I.pos);
}
/******************************************************************************/
// HULL / DOMAIN
/******************************************************************************/
#if MODEL>=SM_4
HSData HSConstant(InputPatch<VS_PS,3> I) {return GetHSData(I[0].pos, I[1].pos, I[2].pos, I[0].mtrx[2], I[1].mtrx[2], I[2].mtrx[2]);}
[maxtessfactor(5.0)]
[domain("tri")]
[partitioning("fractional_odd")] // use 'odd' because it supports range from 1.0 ('even' supports range from 2.0)
[outputtopology("triangle_cw")]
[patchconstantfunc("HSConstant")]
[outputcontrolpoints(3)]
VS_PS HS
(
   InputPatch<VS_PS,3> I, UInt cp_id:SV_OutputControlPointID,
   PARAMS
)
{
   VS_PS O;
                                               O.pos     =I[cp_id].pos     ;
   if(materials<=1 /*|| !mtrl_blend*/ || color)O.col     =I[cp_id].col     ;
                                               O.vel     =I[cp_id].vel     ;
   if(textures || detail                      )O.tex     =I[cp_id].tex     ;
   if(light_map                               )O.tex_l   =I[cp_id].tex_l   ;
   if(materials>1                             )O.material=I[cp_id].material;
   if(fx==FX_GRASS                            )O.fade_out=I[cp_id].fade_out;
   if(bump_mode==SBUMP_FLAT                   )O.mtrx[2] =I[cp_id].mtrx[2] ;
   if(bump_mode> SBUMP_FLAT                   )O.mtrx    =I[cp_id].mtrx    ;

   if((bump_mode>=SBUMP_PARALLAX_MIN && bump_mode<=SBUMP_PARALLAX_MAX)
   || (bump_mode==SBUMP_RELIEF       && !RELIEF_TAN_POS              ))O._tpos=I[cp_id]._tpos;

   return O;
}
/******************************************************************************/
[domain("tri")]
void DS
(
   HSData hs_data, const OutputPatch<VS_PS,3> I, Vec B:SV_DomainLocation,

   out VS_PS O,
   out Vec4  O_vtx:POSITION,

   PARAMS
)
{
   if(materials<=1 /*|| !mtrl_blend*/ || color)O.col     =I[0].col     *B.z + I[1].col     *B.x + I[2].col     *B.y;
                                               O.vel     =I[0].vel     *B.z + I[1].vel     *B.x + I[2].vel     *B.y;
   if(textures || detail                      )O.tex     =I[0].tex     *B.z + I[1].tex     *B.x + I[2].tex     *B.y;
   if(light_map                               )O.tex_l   =I[0].tex_l   *B.z + I[1].tex_l   *B.x + I[2].tex_l   *B.y;
   if(materials>1                             )O.material=I[0].material*B.z + I[1].material*B.x + I[2].material*B.y;
   if(fx==FX_GRASS                            )O.fade_out=I[0].fade_out*B.z + I[1].fade_out*B.x + I[2].fade_out*B.y;

   if(bump_mode>SBUMP_FLAT)
   {
      O.mtrx[0]=I[0].mtrx[0]*B.z + I[1].mtrx[0]*B.x + I[2].mtrx[0]*B.y;
      O.mtrx[1]=I[0].mtrx[1]*B.z + I[1].mtrx[1]*B.x + I[2].mtrx[1]*B.y;
      //mtrx[2] is handled below
   }

   if((bump_mode>=SBUMP_PARALLAX_MIN && bump_mode<=SBUMP_PARALLAX_MAX)
   || (bump_mode==SBUMP_RELIEF       && !RELIEF_TAN_POS              ))O._tpos=I[0]._tpos*B.z + I[1]._tpos*B.x + I[2]._tpos*B.y;

   SetDSPosNrm(O.pos, O.mtrx[2], I[0].pos, I[1].pos, I[2].pos, I[0].mtrx[2], I[1].mtrx[2], I[2].mtrx[2], B, hs_data, false, 0);
   O_vtx=Project(O.pos);
}
#endif
/******************************************************************************/
// TECHNIQUES
/******************************************************************************/
CUSTOM_TECHNIQUE
/******************************************************************************/
