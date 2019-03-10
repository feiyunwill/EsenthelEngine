/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define FUR_LAYERS         48 // this was the smallest value that produced good results when zoomed to a fur object
#define DRAW_LAYERS_NUM    (DEBUG && 0)
#define SUPPORT_FUR_CREATE (DEBUG && 0)
/******************************************************************************/
static ImagePtr     FurCol;
static ShaderParam *FurStep;
static ShaderImage *FurLight;
/******************************************************************************/
#if SUPPORT_FUR_CREATE
#pragma message("!! Warning: Use this only for debugging !!")
static void SetupFur(Int res=256)
{
   Randomizer random(UIDZero);
   static Image img; img.create2D(res, res, IMAGE_R8, 0, false); if(img.lock(LOCK_WRITE))
   {
      FREPD(y, img.h())
      FREPD(x, img.w())img.pixB(x, y)=random(256);
      img.unlock().updateMipMaps();
      if(1)img.save("C:/Esenthel/Data/Img/fur.img");
   }
   GetShaderImage("FurCol")->set(img);
}
#endif
/******************************************************************************/
void InitFur()
{
   if(D.canDraw() && FurCol.get("Img/fur.img"))GetShaderImage("FurCol")->set(FurCol());
   FurStep =GetShaderParam("FurStep");
   FurLight=GetShaderImage("FurLight");
}
void PrepareFur()
{
   FurLight->set(Renderer._lum_1s);
#if SUPPORT_FUR_CREATE
   if(Kb.bp(KB_1))SetupFur(8);
   if(Kb.bp(KB_2))SetupFur(16);
   if(Kb.bp(KB_3))SetupFur(32);
   if(Kb.bp(KB_4))SetupFur(64);
   if(Kb.bp(KB_5))SetupFur(128);
   if(Kb.bp(KB_6))SetupFur(256);
   if(Kb.bp(KB_0))SetupFur(); // default
#endif
}
/******************************************************************************/
#define FUR_VEL_FIX (WINDOWS && GL) // Fur Shader does not ouput velocity to second RT, however for some reason it will set the same value as the source color, #BlendRT TODO: check again in the future (was tested and works OK on Android)
void DrawFur(C MeshRender &mshr, Shader &shader, Flt scale)
{
#if FUR_VEL_FIX
   D.colWrite(0, 1); // disable writing
#endif
   scale=((scale>0) ? Max(1, 0.5f/scale) : 1); // !! scale can be <=0 if behind camera !!
#if DRAW_LAYERS_NUM
   Int layers=0;
#endif
   for(Flt f=scale; f<=FUR_LAYERS-0.5f; f+=scale) // when f==FUR_LAYERS then that layer will be completely transparent, so stop 0.5 before
   {
   #if DRAW_LAYERS_NUM
      layers++;
   #endif
      Vec2 s; s.x=f/FUR_LAYERS; s.y=s.x+1; FurStep->set(s);
      shader.commit ();
      mshr  .drawFur();
   }
#if FUR_VEL_FIX
   D.colWrite(COL_WRITE_RGBA, 1); // restore writing
#endif
#if DRAW_LAYERS_NUM
   D.text(0, D.h()*-0.9f, S+"Fur layers:"+layers);
#endif
}
/******************************************************************************/
}
/******************************************************************************/
