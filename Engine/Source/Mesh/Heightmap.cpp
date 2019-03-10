/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   Material Indexes can be set in any of the first 3 slots, example: 0, 15, 0

   Material Indexes for MtrlCombo's however need to be sorted: 15, 7, 0, 0 (from biggest to smallest),
      so there won't be MtrlCombo's with same materials but different order (like 7, 15, 0, 0)

   Color          image can be IMAGE_R8G8B8  , IMAGE_F32_3, IMAGE_NONE (all white)
   Material Blend image can be IMAGE_R8G8B8A8, IMAGE_F32_4

/******************************************************************************/
#define LODS           4
#define MATERIAL2_EPS  0.01f // was 0.055f but smaller value is needed now because of multi-material per-pixel blending, low value is also important for 2 materials different in brightness, for example dark dirt vs bright snow
#define MATERIAL3_EPS  0.01f // was 0.055f but smaller value is needed now because of multi-material per-pixel blending, low value is also important for 2 materials different in brightness, for example dark dirt vs bright snow
#define MATERIAL4_EPS  0.01f // was 0.055f but smaller value is needed now because of multi-material per-pixel blending, low value is also important for 2 materials different in brightness, for example dark dirt vs bright snow
#define VTX_HEIGHTMAP  1
#define VTX_COMPRESS   1
#define MTRL_BLEND_HP  0 // 0-faster and smaller memory usage
#define VMC_CONTINUOUS 0 // 0-faster !! enabling makes building slower 22fps vs 20fps so keep at zero !! this was an attempt to remove dynamic memory allocation by holding one continuous buffer for vtx mtrl combos
/******************************************************************************/
ASSERT(MAX_HM_RES<=129); // various places use UShort to limit for 16-bit (including 'MtrlCombo.vtxs,tris', 'VtxMtrlCombo.mc_vtx_index')
/******************************************************************************/
static void AdjustSharpness(Flt &sharpness, Int count)
{
   if(count)sharpness/=count;
#if DEBUG
 //LogN(S+sharpness);
   // test results:
   // min 0.0001
   // avg 0.0014
   // max 0.0064
#endif
   Flt avg=0.0014f;
   sharpness=Mid(Pow(sharpness/avg, 0.33f), 0.6f, 2.0f);
}
static Flt GetLodDist(Int lod_index, Flt sharpness)
{
   return (1<<lod_index) * ((lod_index==3) ? 1.5f : 1.0f) * sharpness;
}
static VecB ColorB(C Image &image, Int x, Int y) // !! assumes that 'x, y' are in range !!
{
   switch(image.hwType())
   {
      case IMAGE_R8G8B8: return image.pixB3(x, y);
      case IMAGE_F32_3 :
      {
         Vec c=image.pixF3(x, y)*255;
         return VecB(Mid(RoundPos(c.x), 0, 255),
                     Mid(RoundPos(c.y), 0, 255),
                     Mid(RoundPos(c.z), 0, 255));
      }
   }
   return 0;
}
static Vec4 MtrlBlendF(C Image &image, Int x, Int y) // !! assumes that 'x, y' are in range !!
{
   switch(image.hwType())
   {
      case IMAGE_F32_4   :       return image.pixF4(x, y);
      case IMAGE_R8G8B8A8: {C VecB4 &mb=image.pixB4(x, y); return Vec4(mb.x/255.0f, mb.y/255.0f, mb.z/255.0f, mb.w/255.0f);}
   }
   return 0;
}
static VecB4 MtrlBlendB(C Image &image, Int x, Int y) // !! assumes that 'x, y' are in range !!
{
   switch(image.hwType())
   {
      case IMAGE_R8G8B8A8: return image.pixB4(x, y);
      case IMAGE_F32_4   :
      {
         Vec4 mb=image.pixF4(x, y); if(Flt sum=mb.sum())mb*=255/sum;
         return VecB4(Mid(RoundPos(mb.x), 0, 255),
                      Mid(RoundPos(mb.y), 0, 255),
                      Mid(RoundPos(mb.z), 0, 255),
                      Mid(RoundPos(mb.w), 0, 255));
      }
   }
   return 0;
}
#define SHARPNESS  6 // the higher sharpness, the sharper vtx normals of LOD, however it also causes noise
#define SHARPNESS2 (SHARPNESS*SHARPNESS)
static const Int DownSizeWeights[3][3]=
{
   {65536/SHARPNESS2, 65536/SHARPNESS , 65536/SHARPNESS2},
   {65536/SHARPNESS , 65536/1         , 65536/SHARPNESS },
   {65536/SHARPNESS2, 65536/SHARPNESS , 65536/SHARPNESS2},
};
/******************************************************************************/
// MANAGE
/******************************************************************************/
Heightmap& Heightmap::del()
{
  _height    .del();
  _color     .del();
  _mtrl_index.del();
  _mtrl_blend.del();
  _materials .del();
   return T;
}
/******************************************************************************/
Heightmap& Heightmap::create(Int res, Flt height, C MaterialPtr &material, Bool align_height_to_neighbors, C Heightmap *h_l, C Heightmap *h_r, C Heightmap *h_b, C Heightmap *h_f, C Heightmap *h_lb, C Heightmap *h_lf, C Heightmap *h_rb, C Heightmap *h_rf)
{
   if(res>0)
   {
      Clamp(res, 2, MAX_HM_RES); res=NearestPow2(res)|1; Int res1=res-1;

     _height    .createSoft(res, res, 1, IMAGE_F32     );
     _mtrl_index.createSoft(res, res, 1, IMAGE_R8G8B8A8);
     _mtrl_blend.createSoft(res, res, 1, IMAGE_R8G8B8A8);
     _color     .del  ();
     _materials .clear();

      VecB4 index(getMaterialIndex0(material), 0, 0, 0),
            blend(index.x ? 255 : 0          , 0, 0, 0); // if no material (0=null) is selected then set blending to complete zero to be instantly changed when applying a new material later

      // material images
      REPD(y, res)REPD(x, res)_mtrl_index.pixB4(x, y)=index;
      REPD(y, res)REPD(x, res)_mtrl_blend.pixB4(x, y)=blend;

      // align to neighbors
      if(!h_l && !h_r && !h_b && !h_f && !h_lb && !h_lf && !h_rb && !h_rf) // no neighbors
      {
         REPD(y, res)
         REPD(x, res)_height.pixF(x, y)=height;
      }else // align to neighbors
      { 
         // height
         if(!align_height_to_neighbors)
         {
            REPD(y, res)
            REPD(x, res)_height.pixF(x, y)=height;
         }else
         {
            // clear borders
            REP(res)
            {
              _height.pixF(i, 0   )=height;
              _height.pixF(i, res1)=height;
              _height.pixF(0   , i)=height;
              _height.pixF(res1, i)=height;
            }

            // set borders from neighbors
            if( h_l )REP(res)_height.pixF(   0,    i)=h_l ->_height.pixF(res1,    i);
            if( h_r )REP(res)_height.pixF(res1,    i)=h_r ->_height.pixF(   0,    i);
            if( h_b )REP(res)_height.pixF(   i,    0)=h_b ->_height.pixF(   i, res1);
            if( h_f )REP(res)_height.pixF(   i, res1)=h_f ->_height.pixF(   i,    0);
            if( h_lb)        _height.pixF(   0,    0)=h_lb->_height.pixF(res1, res1);
            if( h_lf)        _height.pixF(   0, res1)=h_lf->_height.pixF(res1,    0);
            if( h_rb)        _height.pixF(res1,    0)=h_rb->_height.pixF(   0, res1);
            if( h_rf)        _height.pixF(res1, res1)=h_rf->_height.pixF(   0,    0);
            if(!h_l ){Flt b= _height.pixF(   0,    0),    f=_height.pixF(   0, res1); REP(res)_height.pixF(   0, i)=Lerp(b, f, i/Flt(res1));}
            if(!h_r ){Flt b= _height.pixF(res1,    0),    f=_height.pixF(res1, res1); REP(res)_height.pixF(res1, i)=Lerp(b, f, i/Flt(res1));}
            if(!h_b ){Flt l= _height.pixF(   0,    0),    r=_height.pixF(res1,    0); REP(res)_height.pixF(i,    0)=Lerp(l, r, i/Flt(res1));}
            if(!h_f ){Flt l= _height.pixF(   0, res1),    r=_height.pixF(res1, res1); REP(res)_height.pixF(i, res1)=Lerp(l, r, i/Flt(res1));}

            // blend center
            for(Int y=1; y<res1; y++)
            for(Int x=1; x<res1; x++)
            {
               Int sx, sy;
               Flt h=0, power=0, p;
               sx=   0; sy=   y; p=1.0f/Dist2(x-sx, y-sy); h+=p*_height.pixF(sx, sy); power+=p;
               sx=res1; sy=   y; p=1.0f/Dist2(x-sx, y-sy); h+=p*_height.pixF(sx, sy); power+=p;
               sx=   x; sy=   0; p=1.0f/Dist2(x-sx, y-sy); h+=p*_height.pixF(sx, sy); power+=p;
               sx=   x; sy=res1; p=1.0f/Dist2(x-sx, y-sy); h+=p*_height.pixF(sx, sy); power+=p;
              _height.pixF(x, y)=h/power; // 'power' will never be zero in this case
            }
         }

         // material
         {
            if(h_l)REP(res)
            {
               VecB4 &index=_mtrl_index.pixB4(0, i); index=h_l->_mtrl_index.pixB4(res1, i);
               index.x=getMaterialIndex0(h_l->material(index.x));
               index.y=getMaterialIndex0(h_l->material(index.y));
               index.z=getMaterialIndex0(h_l->material(index.z));
               index.w=getMaterialIndex0(h_l->material(index.w));
              _mtrl_blend.pixB4(0, i)=MtrlBlendB(h_l->_mtrl_blend, res1, i);
            }
            if(h_r)REP(res)
            {
               VecB4 &index=_mtrl_index.pixB4(res1, i); index=h_r->_mtrl_index.pixB4(0, i);
               index.x=getMaterialIndex0(h_r->material(index.x));
               index.y=getMaterialIndex0(h_r->material(index.y));
               index.z=getMaterialIndex0(h_r->material(index.z));
               index.w=getMaterialIndex0(h_r->material(index.w));
              _mtrl_blend.pixB4(res1, i)=MtrlBlendB(h_r->_mtrl_blend, 0, i);
            }
            if(h_b)REP(res)
            {
               VecB4 &index=_mtrl_index.pixB4(i, 0); index=h_b->_mtrl_index.pixB4(i, res1);
               index.x=getMaterialIndex0(h_b->material(index.x));
               index.y=getMaterialIndex0(h_b->material(index.y));
               index.z=getMaterialIndex0(h_b->material(index.z));
               index.w=getMaterialIndex0(h_b->material(index.w));
              _mtrl_blend.pixB4(i, 0)=MtrlBlendB(h_b->_mtrl_blend, i, res1);
            }
            if(h_f)REP(res)
            {
               VecB4 &index=_mtrl_index.pixB4(i, res1); index=h_f->_mtrl_index.pixB4(i, 0);
               index.x=getMaterialIndex0(h_f->material(index.x));
               index.y=getMaterialIndex0(h_f->material(index.y));
               index.z=getMaterialIndex0(h_f->material(index.z));
               index.w=getMaterialIndex0(h_f->material(index.w));
              _mtrl_blend.pixB4(i, res1)=MtrlBlendB(h_f->_mtrl_blend, i, 0);
            }
            if(h_lb)
            {
               VecB4 &index=_mtrl_index.pixB4(0, 0); index=h_lb->_mtrl_index.pixB4(res1, res1);
               index.x=getMaterialIndex0(h_lb->material(index.x));
               index.y=getMaterialIndex0(h_lb->material(index.y));
               index.z=getMaterialIndex0(h_lb->material(index.z));
               index.w=getMaterialIndex0(h_lb->material(index.w));
              _mtrl_blend.pixB4(0, 0)=MtrlBlendB(h_lb->_mtrl_blend, res1, res1);
            }
            if(h_rb)
            {
               VecB4 &index=_mtrl_index.pixB4(res1, 0); index=h_rb->_mtrl_index.pixB4(0, res1);
               index.x=getMaterialIndex0(h_rb->material(index.x));
               index.y=getMaterialIndex0(h_rb->material(index.y));
               index.z=getMaterialIndex0(h_rb->material(index.z));
               index.w=getMaterialIndex0(h_rb->material(index.w));
              _mtrl_blend.pixB4(res1, 0)=MtrlBlendB(h_rb->_mtrl_blend, 0, res1);
            }
            if(h_lf)
            {
               VecB4 &index=_mtrl_index.pixB4(0, res1); index=h_lf->_mtrl_index.pixB4(res1, 0);
               index.x=getMaterialIndex0(h_lf->material(index.x));
               index.y=getMaterialIndex0(h_lf->material(index.y));
               index.z=getMaterialIndex0(h_lf->material(index.z));
               index.w=getMaterialIndex0(h_lf->material(index.w));
              _mtrl_blend.pixB4(0, res1)=MtrlBlendB(h_lf->_mtrl_blend, res1, 0);
            }
            if(h_rf)
            {
               VecB4 &index=_mtrl_index.pixB4(res1, res1); index=h_rf->_mtrl_index.pixB4(0, 0);
               index.x=getMaterialIndex0(h_rf->material(index.x));
               index.y=getMaterialIndex0(h_rf->material(index.y));
               index.z=getMaterialIndex0(h_rf->material(index.z));
               index.w=getMaterialIndex0(h_rf->material(index.w));
              _mtrl_blend.pixB4(res1, res1)=MtrlBlendB(h_rf->_mtrl_blend, 0, 0);
            }
         }
      }
   }else del();
   return T;
}
/******************************************************************************/
Heightmap& Heightmap::create(C Heightmap &src)
{
   if(this!=&src)
   {
      src._height    .copyTry(_height    );
      src._color     .copyTry(_color     , -1, -1, -1, IMAGE_R8G8B8);
      src._mtrl_index.copyTry(_mtrl_index);
      src._mtrl_blend.copyTry(_mtrl_blend, -1, -1, -1, IMAGE_R8G8B8A8);

     _materials=src._materials;
   }
   return T;
}
/******************************************************************************/
void Heightmap::createFromQuarter(C Heightmap &src, Bool right, Bool forward, C Game::WorldSettings &settings)
{
   src._height    .crop(_height    , right ? (src._height    .w()-1)/2 : 0, forward ? (src._height    .h()-1)/2 : 0, (src._height    .w()-1)/2+1, (src._height    .h()-1)/2+1);
   src._color     .crop(_color     , right ? (src._color     .w()-1)/2 : 0, forward ? (src._color     .h()-1)/2 : 0, (src._color     .w()-1)/2+1, (src._color     .h()-1)/2+1);
   src._mtrl_index.crop(_mtrl_index, right ? (src._mtrl_index.w()-1)/2 : 0, forward ? (src._mtrl_index.h()-1)/2 : 0, (src._mtrl_index.w()-1)/2+1, (src._mtrl_index.h()-1)/2+1);
   src._mtrl_blend.crop(_mtrl_blend, right ? (src._mtrl_blend.w()-1)/2 : 0, forward ? (src._mtrl_blend.h()-1)/2 : 0, (src._mtrl_blend.w()-1)/2+1, (src._mtrl_blend.h()-1)/2+1);

  _materials=src._materials;

   REPD(y, _height.h())
   REPD(x, _height.w())_height.pixF(x, y)*=2; // heightmap scaling is proportional to area size, so we need to adjust it

   resize(settings.hmRes());
}
struct HeightmapQuad
{
 C Heightmap *h;
   Int        x, y;
};
void Heightmap::createFromQuad(C Heightmap *lb, C Heightmap *rb, C Heightmap *lf, C Heightmap *rf, C Game::WorldSettings &settings)
{
  _materials.clear();

   // clear all images at start, and overwrite only if we have source data, this is because the heightmaps overlap each other in 1 pixel, and when clearing quarter without heightmap we could potentially erase 1 pixel line data from other
  _height    .createSoft(settings.hmRes(), settings.hmRes(), 1, IMAGE_F32     ); _height    .clear();
  _mtrl_index.createSoft(settings.hmRes(), settings.hmRes(), 1, IMAGE_R8G8B8A8); _mtrl_index.clear();
  _mtrl_blend.createSoft(settings.hmRes(), settings.hmRes(), 1, IMAGE_R8G8B8A8); _mtrl_blend.clear();
   if((lb && lb->_color.is()) || (rb && rb->_color.is()) || (lf && lf->_color.is()) || (rf && rf->_color.is()))
   {
     _color.createSoft(settings.hmRes(), settings.hmRes(), 1, IMAGE_R8G8B8);
      REPD(y, _color.h())
      REPD(x, _color.w())_color.pixB3(x, y)=255;
   }else _color.del();

   Heightmap temp; // keep outside of the loop
   const Int temp_size=(settings.hmRes()-1)/2+1;

   HeightmapQuad hqs[]=
   {
      {lb, 0, 0},
      {rb, 1, 0},
      {lf, 0, 1},
      {rf, 1, 1},
   };
   REPA(hqs)
   {
    C HeightmapQuad &hq=hqs[i]; if(hq.h)
      {
         Int ofs_x=(hq.x ? temp_size-1 : 0),
             ofs_y=(hq.y ? temp_size-1 : 0);
         temp.create(*hq.h); temp.resize(temp_size);
         REPD(y, temp_size)
         REPD(x, temp_size)
         {
            Int dest_x=ofs_x+x,
                dest_y=ofs_y+y;
                                                         VecB4 mi=           temp._mtrl_index.pixB4 (x, y);
                                _height    .pixF (dest_x, dest_y)=           temp._height    .pixF  (x, y)/2; // heightmap scaling is proportional to area size, so we need to adjust it
            if(temp._color.is())_color     .pixB3(dest_x, dest_y)=    ColorB(temp._color     ,       x, y);
                                _mtrl_blend.pixB4(dest_x, dest_y)=MtrlBlendB(temp._mtrl_blend,       x, y);
                                _mtrl_index.pixB4(dest_x, dest_y).set(getMaterialIndex0(temp.material(mi.x)),
                                                                      getMaterialIndex0(temp.material(mi.y)),
                                                                      getMaterialIndex0(temp.material(mi.z)),
                                                                      getMaterialIndex0(temp.material(mi.w)));
         }
      }//else clear - don't do this, because we could erase 1 pixel line of other heightmap which was already set, instead, all heightmap data is cleared at start
   }
}
/******************************************************************************/
// GET
/******************************************************************************/
Bool Heightmap::is        ()C {return _height.is();}
Int  Heightmap::resolution()C {return _height.w ();}
Bool Heightmap::hasColor  ()C {return _color .is();}

UInt Heightmap::memUsage()C
{
   return _height    .memUsage()
         +_color     .memUsage()
         +_mtrl_index.memUsage()
         +_mtrl_blend.memUsage()
         +_materials .memUsage();
}

Flt  Heightmap::height      (Int x, Int y            )C {return (InRange(x, _height.w()) && InRange(y, _height.h())) ? _height.pixF        (x, y) : 0   ;}
Flt  Heightmap::heightLinear(Flt x, Flt y            )C {return                                                        _height.pixelFLinear(x, y)       ;}
void Heightmap::height      (Int x, Int y, Flt height)  {if     (InRange(x, _height.w()) && InRange(y, _height.h()))   _height.pixF        (x, y)=height;}
/******************************************************************************/
Color Heightmap::color (Int x, Int y)C {return (InRange(x, _color.w()) && InRange(y, _color.h())) ? _color.color (x, y)     : WHITE ;}
Vec   Heightmap::colorF(Int x, Int y)C {return (InRange(x, _color.w()) && InRange(y, _color.h())) ? _color.colorF(x, y).xyz : VecOne;}

void Heightmap::color(Int x, Int y, C Color &color)
{
   if(InRange(x, _height.w()) && InRange(y, _height.h())) // check '_height' size because '_color' may not exist yet, as it is optional
   {
      if(!_color.is() && (color.r!=255 || color.g!=255 || color.b!=255)) // create if doesn't exist yet but is needed
      {
        _color.createSoftTry(_height.w(), _height.h(), 1, IMAGE_R8G8B8);
         REPD(y, _color.h())
         REPD(x, _color.w())_color.pixB3(x, y)=255;
      }
     _color.color(x, y, color);
   }
}
void Heightmap::colorF(Int x, Int y, C Vec &color)
{
   if(InRange(x, _height.w()) && InRange(y, _height.h())) // check '_height' size because '_color' may not exist yet, as it is optional
   {
      if(!_color.is() && (color.x!=1 || color.y!=1 || color.z!=1)) // create if doesn't exist yet but is needed
      {
        _color.createSoftTry(_height.w(), _height.h(), 1, IMAGE_F32_3);
         REPD(y, _color.h())
         REPD(x, _color.w())_color.pixF3(x, y)=1;
      }
      if(_color.is())
      {
         if(_color.type()!=IMAGE_F32_3)_color.copy(_color, -1, -1, -1, IMAGE_F32_3); // convert to high precision
        _color.pixF3(x, y)=color;
      }
   }
}
/******************************************************************************/
void Heightmap::mtrlBlendHP()
{
   if(_mtrl_blend.hwType()==IMAGE_R8G8B8A8)
   {
      Image   temp; temp.create(_mtrl_blend.w(), _mtrl_blend.h(), 1, IMAGE_F32_4, IMAGE_SOFT, 1, false); // use 'create' to have 'Exit' on fail, because the methods that use 'mtrlBlendHP', assume that it succeeded
      REPD(y, temp.h())
      REPD(x, temp.w())
      {
         VecB4 mb=_mtrl_blend.pixB4(x, y);
         temp.pixF4(x, y).set(mb.x/255.0f, mb.y/255.0f, mb.z/255.0f, mb.w/255.0f);
      }
      Swap(_mtrl_blend, temp);
   }
}
/******************************************************************************/
Bool Heightmap::getMaterial(Int x, Int y, VecB4 &mtrl_index, VecB4 &mtrl_blend)C
{
   if(InRange(x, _mtrl_index.w())
   && InRange(y, _mtrl_index.h()))
   {
      mtrl_index=           _mtrl_index.pixB4(x, y);
      mtrl_blend=MtrlBlendB(_mtrl_blend,      x, y);
      return true;
   }
   mtrl_index.zero(); mtrl_blend.zero(); return false;
}
Bool Heightmap::getMaterial(Int x, Int y, VecB4 &mtrl_index, Vec4 &mtrl_blend)C
{
   if(InRange(x, _mtrl_index.w())
   && InRange(y, _mtrl_index.h()))
   {
      mtrl_index=           _mtrl_index.pixB4(x, y);
      mtrl_blend=MtrlBlendF(_mtrl_blend,      x, y);
      return true;
   }
   mtrl_index.zero(); mtrl_blend.zero(); return false;
}
/******************************************************************************/
Bool Heightmap::getMaterial(Int x, Int y, MaterialPtr &m0, MaterialPtr &m1, MaterialPtr &m2, MaterialPtr &m3, VecB4 &mtrl_blend)C
{
   if(InRange(x, _mtrl_index.w())
   && InRange(y, _mtrl_index.h()))
   {
             mtrl_blend=MtrlBlendB(_mtrl_blend,      x, y);
    C VecB4 &mtrl_index=           _mtrl_index.pixB4(x, y);
      if(InRange(mtrl_index.x, _materials))m0=_materials[mtrl_index.x];else m0=null;
      if(InRange(mtrl_index.y, _materials))m1=_materials[mtrl_index.y];else m1=null;
      if(InRange(mtrl_index.z, _materials))m2=_materials[mtrl_index.z];else m2=null;
      if(InRange(mtrl_index.w, _materials))m3=_materials[mtrl_index.w];else m3=null;
      return true;
   }
   m0=null; m1=null; m2=null; m3=null; mtrl_blend.zero(); return false;
}
Bool Heightmap::getMaterial(Int x, Int y, MaterialPtr &m0, MaterialPtr &m1, MaterialPtr &m2, MaterialPtr &m3, Vec4 &mtrl_blend)C
{
   if(InRange(x, _mtrl_index.w())
   && InRange(y, _mtrl_index.h()))
   {
             mtrl_blend=MtrlBlendF(_mtrl_blend,      x, y);
    C VecB4 &mtrl_index=           _mtrl_index.pixB4(x, y);
      if(InRange(mtrl_index.x, _materials))m0=_materials[mtrl_index.x];else m0=null;
      if(InRange(mtrl_index.y, _materials))m1=_materials[mtrl_index.y];else m1=null;
      if(InRange(mtrl_index.z, _materials))m2=_materials[mtrl_index.z];else m2=null;
      if(InRange(mtrl_index.w, _materials))m3=_materials[mtrl_index.w];else m3=null;
      return true;
   }
   m0=null; m1=null; m2=null; m3=null; mtrl_blend.zero(); return false;
}
/******************************************************************************/
void Heightmap::setMaterial(Int x, Int y, C VecB4 &mtrl_index, C VecB4 &mtrl_blend)
{
   if(InRange(x, _mtrl_index.w())
   && InRange(y, _mtrl_index.h()))
   {
      VecB4 &mi=_mtrl_index.pixB4(x, y);
      switch(_mtrl_blend.hwType())
      {
         case IMAGE_R8G8B8A8:
         {
            VecB4 &mb=_mtrl_blend.pixB4(x, y);
            if(InRange(mtrl_index.x, _materials)){mi.x=mtrl_index.x; mb.x=mtrl_blend.x;}else{mi.x=0; mb.x=0;}
            if(InRange(mtrl_index.y, _materials)){mi.y=mtrl_index.y; mb.y=mtrl_blend.y;}else{mi.y=0; mb.y=0;}
            if(InRange(mtrl_index.z, _materials)){mi.z=mtrl_index.z; mb.z=mtrl_blend.z;}else{mi.z=0; mb.z=0;}
            if(InRange(mtrl_index.w, _materials)){mi.w=mtrl_index.w; mb.w=mtrl_blend.w;}else{mi.w=0; mb.w=0;}
            // normalizing blends would be slow, so just keep as it is, because they will be normalized when building
         }break;

         case IMAGE_F32_4:
         {
            Vec4 &mb=_mtrl_blend.pixF4(x, y);
            if(InRange(mtrl_index.x, _materials)){mi.x=mtrl_index.x; mb.x=mtrl_blend.x;}else{mi.x=0; mb.x=0;}
            if(InRange(mtrl_index.y, _materials)){mi.y=mtrl_index.y; mb.y=mtrl_blend.y;}else{mi.y=0; mb.y=0;}
            if(InRange(mtrl_index.z, _materials)){mi.z=mtrl_index.z; mb.z=mtrl_blend.z;}else{mi.z=0; mb.z=0;}
            if(InRange(mtrl_index.w, _materials)){mi.w=mtrl_index.w; mb.w=mtrl_blend.w;}else{mi.w=0; mb.w=0;}
            if(Flt sum=mb.sum())mb/=sum; // normalize blends
         }break;
      }
   }
}
void Heightmap::setMaterial(Int x, Int y, C VecB4 &mtrl_index, C Vec4 &mtrl_blend)
{
   if(InRange(x, _mtrl_index.w())
   && InRange(y, _mtrl_index.h()))
   {
      mtrlBlendHP();
      VecB4 &mi=_mtrl_index.pixB4(x, y);
      Vec4  &mb=_mtrl_blend.pixF4(x, y);
      if(InRange(mtrl_index.x, _materials)){mi.x=mtrl_index.x; mb.x=mtrl_blend.x;}else{mi.x=0; mb.x=0;}
      if(InRange(mtrl_index.y, _materials)){mi.y=mtrl_index.y; mb.y=mtrl_blend.y;}else{mi.y=0; mb.y=0;}
      if(InRange(mtrl_index.z, _materials)){mi.z=mtrl_index.z; mb.z=mtrl_blend.z;}else{mi.z=0; mb.z=0;}
      if(InRange(mtrl_index.w, _materials)){mi.w=mtrl_index.w; mb.w=mtrl_blend.w;}else{mi.w=0; mb.w=0;}
      if(Flt sum=mb.sum())mb/=sum; // normalize blends
   }
}
/******************************************************************************/
void Heightmap::setMaterial(Int x, Int y, C MaterialPtr &m0, C MaterialPtr &m1, C MaterialPtr &m2, C MaterialPtr &m3, C VecB4 &mtrl_blend)
{
   if(InRange(x, _mtrl_index.w())
   && InRange(y, _mtrl_index.h()))
   {
             _mtrl_index.pixB4(x, y).set(getMaterialIndex0(m0), getMaterialIndex0(m1), getMaterialIndex0(m2), getMaterialIndex0(m3));
      switch(_mtrl_blend.hwType())
      {
         case IMAGE_R8G8B8A8:
         {
            VecB4 &mb=_mtrl_blend.pixB4(x, y); mb=mtrl_blend;
            // normalizing blends would be slow, so just keep as it is, because they will be normalized when building
         }break;

         case IMAGE_F32_4:
         {
            Vec4 &mb=_mtrl_blend.pixF4(x, y); mb=mtrl_blend;
            if(Flt sum=mb.sum())mb/=sum; // normalize blends
         }break;
      }
   }
}
void Heightmap::setMaterial(Int x, Int y, C MaterialPtr &m0, C MaterialPtr &m1, C MaterialPtr &m2, C MaterialPtr &m3, C Vec4 &mtrl_blend)
{
   if(InRange(x, _mtrl_index.w())
   && InRange(y, _mtrl_index.h()))
   {
      mtrlBlendHP();
               _mtrl_index.pixB4(x, y).set(getMaterialIndex0(m0), getMaterialIndex0(m1), getMaterialIndex0(m2), getMaterialIndex0(m3));
      Vec4 &mb=_mtrl_blend.pixF4(x, y); mb=mtrl_blend;
      if(Flt sum=mb.sum())mb/=sum; // normalize blends
   }
}
/******************************************************************************/
void Heightmap::addMaterial(Int x, Int y, C MaterialPtr &m, Flt mtrl_blend)
{
   if(InRange(x, _mtrl_index.w())
   && InRange(y, _mtrl_index.h()))
   {
      mtrlBlendHP();
      VecB4 &mi=_mtrl_index.pixB4(x, y);
      Vec4  &mb=_mtrl_blend.pixF4(x, y);
      if(!m) // adding null material means setting a hole, this is how Editor works
      {
         mi.zero();
         mb.zero();
      }else
      {
         Int index=getMaterialIndex(m);
         if( index>0) // skip -1 and 0 null materials
         {
            Int c;
            if(mi.x==index)c=0;else
            if(mi.y==index)c=1;else
            if(mi.z==index)c=2;else
            if(mi.w==index)c=3;else
               c=-1;

            if(c>=0) // material already exists in pixel
            {
               mb.c[c]+=mtrl_blend; // increase its power
            }else // new material
            {
               // replace the least powerful
               c=mb.minI();
               mi.c[c]=index;
               mb.c[c]=mtrl_blend;
            }

            // normalize blends
            if(Flt sum=mb.sum())mb/=sum;

            // remove insignificant materials, this is to prevent too many materials residing in memory while they're unused anymore
            if(mb.x<1.0f/255){mi.x=0; mb.x=0;}
            if(mb.y<1.0f/255){mi.y=0; mb.y=0;}
            if(mb.z<1.0f/255){mi.z=0; mb.z=0;}
            if(mb.w<1.0f/255){mi.w=0; mb.w=0;}
         }
      }
   }
}
/******************************************************************************/
// SET
/******************************************************************************
/*void Heightmap::setMinMaxY(Mesh &mesh) // this function should be multi-threaded safe
{
   Int step=1..4; // can be >1 for approximation
   for(Int y=0; y<_height.h(); y+=step)
   for(Int x=0; x<_height.w(); x+=step)
   {
      Flt h=_height.pixF(x, y);
      if(!x && !y)mesh.box.min.y=mesh.box.max.y=h; // first pixel
      else        mesh.box.includeY(h);            // next  pixels
   }
   mesh.lod_center.y=mesh.box.centerY();
}
void Heightmap::setNormal(Mesh &mesh, Int quality, C Heightmap *h_l, C Heightmap *h_r, C Heightmap *h_b, C Heightmap *h_f) // this function should be multi-threaded safe
{
   Int res=resolution();
   if( res>=2)
   {
      Int step=(1<<quality);
      Flt sharpness=0; Int sharpness_count=0;
      Vec vtx_nrm_map[MAX_HM_RES][MAX_HM_RES]; // [y][x]

    C Image *hl=(h_l ? &h_l->_height : null),
            *hr=(h_r ? &h_r->_height : null),
            *hb=(h_b ? &h_b->_height : null),
            *hf=(h_f ? &h_f->_height : null);

      Int res1 =res-1;
      Flt nrm_y=2.0f/res1;
      for(Int y=0; y<res; y+=step)
      for(Int x=0; x<res; x+=step)
      {
         Flt h=_height.pixF(x, y);

         // normal
         Vec &nrm=vtx_nrm_map[y][x];

         Flt l, r;
         if(x!=   0)l=_height.pixF(x-1, y);else if(hl)l=hl->pixF(res-2, y);else{nrm.x=(_height.pixF(    0, y)-_height.pixF(   1, y))*2; goto processed_x;}
         if(x!=res1)r=_height.pixF(x+1, y);else if(hr)r=hr->pixF(    1, y);else{nrm.x=(_height.pixF(res-2, y)-_height.pixF(res1, y))*2; goto processed_x;}
         nrm.x=l-r;
         sharpness+=Abs(h-Avg(l, r)); sharpness_count++;
      processed_x:

         Flt b, f;
         if(y!=   0)b=_height.pixF(x, y-1);else if(hb)b=hb->pixF(x, res-2);else{nrm.z=(_height.pixF(x,     0)-_height.pixF(x,    1))*2; goto processed_z;}
         if(y!=res1)f=_height.pixF(x, y+1);else if(hf)f=hf->pixF(x,     1);else{nrm.z=(_height.pixF(x, res-2)-_height.pixF(x, res1))*2; goto processed_z;}
         nrm.z=b-f;
         sharpness+=Abs(h-Avg(b, f)); sharpness_count++;
      processed_z:

         nrm.y=nrm_y; nrm.normalize();
      }

      AdjustSharpness(sharpness, sharpness_count);

      FREPD(l, mesh.lods())
      {
         MeshLod &lod=mesh.lod(l);
         lod.dist(GetLodDist(l, sharpness));
         REPA(lod)
         {
            MeshRender &mshr   =lod.parts[i].render;
            Int         ofs_pos=mshr.vtxOfs(VTX_POS),
                        ofs_nrm=mshr.vtxOfs(VTX_NRM);
            if(ofs_pos>=0 && ofs_nrm>=0)if(Byte *vtx=mshr.vtxLock())
            {
               Int   dx     =_height.w(),
                     dy     =_height.h();
               Vec  *vtx_pos=(Vec *)(vtx+ofs_pos);
               Byte *vtx_nrm=(Byte*)(vtx+ofs_nrm);
               REP(mshr.vtxs())
               {
                  Int x=RoundPos(vtx_pos->x*(dx-1)); Clamp(x, 0, res1);
                  Int y=RoundPos(vtx_pos->z*(dy-1)); Clamp(y, 0, res1);

                  if(!mshr.storageCompress())*(Vec  *)vtx_nrm=            vtx_nrm_map[y][x] ;else
                  if( mshr.storageSigned  ())*(VecB4*)vtx_nrm=NrmToSByte4(vtx_nrm_map[y][x]);else
                                             *(VecB4*)vtx_nrm=NrmToUByte4(vtx_nrm_map[y][x]);

                  vtx_pos =(Vec*)(((Byte*)vtx_pos)+mshr.vtxSize());
                  vtx_nrm+=mshr.vtxSize();
               }
               mshr.vtxUnlock();
            }
         }
         SoftenNormalBorders(vtx_nrm_map, res, step<<l);
      }
   }
}
/******************************************************************************/
void Heightmap::clearMaterials() {_materials.clear();}
void Heightmap::cleanMaterials() // this function should be multi-threaded safe
{
   if(_materials.elms()>1)
   {
      Bool    is[256]; ZeroN(is, _materials.elms()); // assume all are unused
      Byte remap[256];

      switch(_mtrl_blend.hwType())
      {
         case IMAGE_R8G8B8A8:
         {
            REPD(y, _mtrl_index.h())
            REPD(x, _mtrl_index.w())
            {
             C VecB4 &mi=_mtrl_index.pixB4(x, y),
                     &mb=_mtrl_blend.pixB4(x, y);
               if(mb.x>0)is[mi.x]=true;
               if(mb.y>0)is[mi.y]=true;
               if(mb.z>0)is[mi.z]=true;
               if(mb.w>0)is[mi.w]=true;
            }
         }break;

         case IMAGE_F32_4:
         {
            REPD(y, _mtrl_index.h())
            REPD(x, _mtrl_index.w())
            {
             C VecB4 &mi=_mtrl_index.pixB4(x, y);
             C Vec4  &mb=_mtrl_blend.pixF4(x, y);
               if(mb.x>0)is[mi.x]=true;
               if(mb.y>0)is[mi.y]=true;
               if(mb.z>0)is[mi.z]=true;
               if(mb.w>0)is[mi.w]=true;
            }
         }break;
      }

      if(_materials.clean(is, remap)) // if any was removed
         REPD(y, _mtrl_index.h())
         REPD(x, _mtrl_index.w())
      {
         VecB4 &mi=_mtrl_index.pixB4(x, y);
         mi.x=remap[mi.x];
         mi.y=remap[mi.y];
         mi.z=remap[mi.z];
         mi.w=remap[mi.w];
         // mtrl blends don't need to be set to zero for removed materials, because materials with blend>0 are never removed
      }
   }
}
void Heightmap::cleanColor()
{
   if(_color.is())
   {
      REPD(y, _color.h())
      REPD(x, _color.w()){Color c=_color.color(x, y); if(c.r!=255 || c.g!=255 || c.b!=255)return;} // if color is not white, then we need the color map, so return without deleting it
     _color.del(); // we don't need to save color (because it's all white) so we can just delete it
   }
}
void Heightmap::clean()
{
   cleanColor    ();
   cleanMaterials();
}
/******************************************************************************/
// BUILD
/******************************************************************************/
static inline VecB4 VtxMtrlBlend(C VecB4 &mtrl_index, C VecB4 &pix_ind, C VecB4 &pix_blend)
{
   VecI4 mtrl_blend(0);

 //if(mtrl_index.x) we can assume that this will always be != 0 since mtrl_index is sorted for mtrl combos
   {
      if(mtrl_index.x==pix_ind.x)mtrl_blend.x+=pix_blend.x;else
      if(mtrl_index.x==pix_ind.y)mtrl_blend.x+=pix_blend.y;else
      if(mtrl_index.x==pix_ind.z)mtrl_blend.x+=pix_blend.z;else
      if(mtrl_index.x==pix_ind.w)mtrl_blend.x+=pix_blend.w;
   }

   if(mtrl_index.y) // checking this will prevent adding material blends to null materials (in case source pixel mtrl_index=null and mtrl_blend>0), this is also a speedup because in most cases vertexes don't have second material, so codes below don't need to be executed
   {
      if(mtrl_index.y==pix_ind.x)mtrl_blend.y+=pix_blend.x;else
      if(mtrl_index.y==pix_ind.y)mtrl_blend.y+=pix_blend.y;else
      if(mtrl_index.y==pix_ind.z)mtrl_blend.y+=pix_blend.z;else
      if(mtrl_index.y==pix_ind.w)mtrl_blend.y+=pix_blend.w;

      // since 'mtrl_index' is sorted, then 'mtrl_index.z' will be set only if 'mtrl_index.y' is, so we can check the following in this scope
      if(mtrl_index.z) // checking this will prevent adding material blends to null materials (in case source pixel mtrl_index=null and mtrl_blend>0), this is also a speedup because in most cases vertexes don't have third material, so codes below don't need to be executed
      {
         if(mtrl_index.z==pix_ind.x)mtrl_blend.z+=pix_blend.x;else
         if(mtrl_index.z==pix_ind.y)mtrl_blend.z+=pix_blend.y;else
         if(mtrl_index.z==pix_ind.z)mtrl_blend.z+=pix_blend.z;else
         if(mtrl_index.z==pix_ind.w)mtrl_blend.z+=pix_blend.w;

         // since 'mtrl_index' is sorted, then 'mtrl_index.w' will be set only if 'mtrl_index.z' is, so we can check the following in this scope
         if(mtrl_index.w) // checking this will prevent adding material blends to null materials (in case source pixel mtrl_index=null and mtrl_blend>0), this is also a speedup because in most cases vertexes don't have third material, so codes below don't need to be executed
         {
            if(mtrl_index.w==pix_ind.x)mtrl_blend.w+=pix_blend.x;else
            if(mtrl_index.w==pix_ind.y)mtrl_blend.w+=pix_blend.y;else
            if(mtrl_index.w==pix_ind.z)mtrl_blend.w+=pix_blend.z;else
            if(mtrl_index.w==pix_ind.w)mtrl_blend.w+=pix_blend.w;
         }
      }
   }

      Flt mul=0;
   if(Int sum=mtrl_blend.sum())mul=255.0f/sum;//else mtrl_blend.x=; since below we're not using 'mtrl_blend.x' then we don't need to set it

   VecB4 b;
   b.w=Mid(RoundPos(mtrl_blend.w*mul), 0, 255);
   b.z=Mid(RoundPos(mtrl_blend.z*mul), 0, 255);
   b.y=Mid(RoundPos(mtrl_blend.y*mul), 0, 255);
   b.x=Mid(255-b.y-b.z-b.w           , 0, 255); // make sure that sum of all channels is 255, do this for the first channel, in case 'mtrl_blend' was 0, normally this would set zero too, based on "Mid(RoundPos(mtrl_blend.x*255), 0, 255)", however we need full blend for the main material so this will set 255 in that case
   return b;
}
static inline VecB4 VtxMtrlBlend(C VecB4 &mtrl_index, C VecB4 &pix_ind, C Vec4 &pix_blend)
{
   Vec4 mtrl_blend(0);

 //if(mtrl_index.x) we can assume that this will always be != 0 since mtrl_index is sorted for mtrl combos
   {
      if(mtrl_index.x==pix_ind.x)mtrl_blend.x+=pix_blend.x;else
      if(mtrl_index.x==pix_ind.y)mtrl_blend.x+=pix_blend.y;else
      if(mtrl_index.x==pix_ind.z)mtrl_blend.x+=pix_blend.z;else
      if(mtrl_index.x==pix_ind.w)mtrl_blend.x+=pix_blend.w;
   }

   if(mtrl_index.y) // checking this will prevent adding material blends to null materials (in case source pixel mtrl_index=null and mtrl_blend>0), this is also a speedup because in most cases vertexes don't have second material, so codes below don't need to be executed
   {
      if(mtrl_index.y==pix_ind.x)mtrl_blend.y+=pix_blend.x;else
      if(mtrl_index.y==pix_ind.y)mtrl_blend.y+=pix_blend.y;else
      if(mtrl_index.y==pix_ind.z)mtrl_blend.y+=pix_blend.z;else
      if(mtrl_index.y==pix_ind.w)mtrl_blend.y+=pix_blend.w;

      // since 'mtrl_index' is sorted, then 'mtrl_index.z' will be set only if 'mtrl_index.y' is, so we can check the following in this scope
      if(mtrl_index.z) // checking this will prevent adding material blends to null materials (in case source pixel mtrl_index=null and mtrl_blend>0), this is also a speedup because in most cases vertexes don't have third material, so codes below don't need to be executed
      {
         if(mtrl_index.z==pix_ind.x)mtrl_blend.z+=pix_blend.x;else
         if(mtrl_index.z==pix_ind.y)mtrl_blend.z+=pix_blend.y;else
         if(mtrl_index.z==pix_ind.z)mtrl_blend.z+=pix_blend.z;else
         if(mtrl_index.z==pix_ind.w)mtrl_blend.z+=pix_blend.w;

         // since 'mtrl_index' is sorted, then 'mtrl_index.w' will be set only if 'mtrl_index.z' is, so we can check the following in this scope
         if(mtrl_index.w) // checking this will prevent adding material blends to null materials (in case source pixel mtrl_index=null and mtrl_blend>0), this is also a speedup because in most cases vertexes don't have third material, so codes below don't need to be executed
         {
            if(mtrl_index.w==pix_ind.x)mtrl_blend.w+=pix_blend.x;else
            if(mtrl_index.w==pix_ind.y)mtrl_blend.w+=pix_blend.y;else
            if(mtrl_index.w==pix_ind.z)mtrl_blend.w+=pix_blend.z;else
            if(mtrl_index.w==pix_ind.w)mtrl_blend.w+=pix_blend.w;
         }
      }
   }

   if(Flt sum=mtrl_blend.sum())
   {
      sum=255/sum;
      mtrl_blend.w*=sum;
      mtrl_blend.z*=sum;
      mtrl_blend.y*=sum;
    //mtrl_blend.x*=sum;     since below we're not using 'mtrl_blend.x' then we don't need to set it
   }//else mtrl_blend.x=255; since below we're not using 'mtrl_blend.x' then we don't need to set it

   VecB4 b;
   b.w=Mid(RoundPos(mtrl_blend.w), 0, 255);
   b.z=Mid(RoundPos(mtrl_blend.z), 0, 255);
   b.y=Mid(RoundPos(mtrl_blend.y), 0, 255);
   b.x=Mid(255-b.y-b.z-b.w       , 0, 255); // make sure that sum of all channels is 255, do this for the first channel, in case 'mtrl_blend' was 0, normally this would set zero too, based on "Mid(RoundPos(mtrl_blend.x*255), 0, 255)", however we need full blend for the main material so this will set 255 in that case
   return b;
}
/******************************************************************************/
struct MtrlCombo // Material Combination
{
   VecB4  mtrl_index  ; // use 4-byte here even if MAX_MTRL==3, because comparing VecB4==VecB4 is faster than VecB==VecB because for VecB4 we can just compare the "UInt u" member
   SByte  ofs_pos     ,
          ofs_nrm     ,
          ofs_color   ,
          ofs_material;
   UShort vtxs        ,
          tris        ;

   void reset() {vtxs=tris=0;}
};
#if VMC_CONTINUOUS
struct VtxMtrlCombo
{
   UShort next_add, // index of next VMC for the same vertex, 0 if none (normally this would be an absolute index, however since there can be ~98K elements, then we would need UInt, but since the vertexes will usually be close together, then we use UShort and use it as relative index), the highest number of VMC is for the first lod, which only uses quads, other LODs can use triangles/edge skirts which could set vertexes first which are further apart, however since they are LOD's then the total number of triangles is smaller, and the next index should be in the range of 64K always
          mtrl_combo, // index of material combo
          mc_vtx_index; // vertex index in that material combo
};
struct Heightmap::BuildMem
{
};
#else
#define VMC_NULL 0xFFFF
struct VtxMtrlCombo
{
   UShort mtrl_combo, // index of material combo, VMC_NULL if none
          mc_vtx_index; // vertex index in that material combo
};
struct VtxMtrlCombos
{
   VtxMtrlCombo vmc[16]; // 8 for edge skirting 0, 10 for edge skirting 1, 16 for edge skirting 2
/* This is the max amount of material combinations per vertex affected by each face
   Worst case scenario is a vertex for LOD near the corner:

   +----+---/+
   |   /|  / |
   |  / | /  |
   | /  | /  |
   |/   | /  |
   <\   | /  |
   | \  | /  |
   |  \ | /  |
   |   \|/   |
   +----+----+    LeftUp   2 tris, RightUp   2 tris   2+2+2+4 = 6+4 = 10
   | ///|\  /|    LeftDown 4 tris, RightDown 2 tris
   </// | \/ |
   |/ / |  | |
   +-<--|--+-+
*/
   void clear()
   {
      vmc[0].mtrl_combo=VMC_NULL; // set at start the first as non-existing
   }
   void setUsedByMtrlCombo(UInt mtrl_combo, Memt<MtrlCombo> &mtrl_combos)
   {
      FREPA(vmc)
      {
         VtxMtrlCombo &v=vmc[i];
         UInt mc=v.mtrl_combo;
         if(  mc==mtrl_combo)return; // found
         if(  mc==VMC_NULL  ) // create new one
         {
            v.mtrl_combo  =mtrl_combo;
            v.mc_vtx_index=mtrl_combos[mtrl_combo].vtxs++;
            if(InRange(i+1, vmc))vmc[i+1].mtrl_combo=VMC_NULL; // set next as non-existing
            return;
         }
      }
      Exit("VMC"); // if there's no room for mtrl combos, then it means the max amount was calculated incorrectly
   }
   Int vtxIndexInMtrlCombo(UInt mtrl_combo)
   {
      FREPA(vmc)
      {
         VtxMtrlCombo &v=vmc[i];
         UInt mc=v.mtrl_combo;
         if(  mc==mtrl_combo)return v.mc_vtx_index;
         if(  mc==VMC_NULL  )break; // reached the end
      }
      Exit("VMC"); return 0; // we shouldn't reach this place
   }
};
struct Heightmap::BuildMem // this is always used 
{
   VtxMtrlCombos vmc[MAX_HM_RES][MAX_HM_RES]; // [y][x]
};
struct Heightmap::BuildMemSoft : Heightmap::BuildMem // !! must inherit from 'BuildMem' !! this is used only for HM_SOFT
{
   Vec vtx_nrm[MAX_HM_RES][MAX_HM_RES]; // [y][x]
};
#endif
/******************************************************************************/
#if MTRL_BLEND_HP // Material Blend types
typedef Flt  MB1;
typedef Flt  MB1L;
typedef Vec4 MBN;
typedef Vec4 MBNL;
#else
typedef Byte  MB1;
typedef Int   MB1L;
typedef VecB4 MBN;
typedef VecI4 MBNL;
#endif

struct Builder
{
   Int   cur_mtrl_combo;
   VecB4 cur_mtrl_index;

   MB1L material_blends[256];

   UShort map_mtrl_combo[MAX_HM_RES-1][MAX_HM_RES-1], // [y][x]
          edg_mtrl_combo[4           ][MAX_HM_RES-1],
          tri_mtrl_combo[4           ][MAX_HM_RES-1];
   Byte   occlusion     [MAX_HM_RES  ][MAX_HM_RES  ]; // [y][x]
   Flt    frac          [MAX_HM_RES  ];
#if VTX_COMPRESS
   VecB4  vtx_nrm       [MAX_HM_RES  ][MAX_HM_RES  ]; // [y][x]
#else
   Vec    vtx_nrm       [MAX_HM_RES  ][MAX_HM_RES  ]; // [y][x]
#endif
 //Vec    vtx_pos       [MAX_HM_RES  ][MAX_HM_RES  ]; // [y][x]
 //Vec    vtx_tan       [MAX_HM_RES  ][MAX_HM_RES  ]; // [y][x]
 //Vec2   vtx_tex       [MAX_HM_RES  ][MAX_HM_RES  ]; // [y][x]
 //VecB4  mtrl_index    [MAX_HM_RES  ][MAX_HM_RES  ]; // [y][x]
   MBN    mtrl_blend    [MAX_HM_RES  ][MAX_HM_RES  ]; // [y][x]

   Memt<MtrlCombo> mtrl_combos;

#if VMC_CONTINUOUS
   UInt         vmc_elms, // number of vmc currently used
                vmc_first[MAX_HM_RES][MAX_HM_RES]; // [y][x] index of the first vmc for a given vertex location, ~0 if none
   VtxMtrlCombo vmc[(MAX_HM_RES-1)*(MAX_HM_RES-1)*2*3]; // (MAX_HM_RES-1)*(MAX_HM_RES-1)=max quads, *2=max tris, *3=max unique mtrl combos per tri vertex = 98K when MAX_HM_RES-1=128
#else
   VtxMtrlCombos (&vmc)[MAX_HM_RES][MAX_HM_RES];
#endif

   Mems<MeshPart> *lod_parts;
   Image          &height, &mtrl_index;

   Builder(Image &height, Image &mtrl_index
   #if !VMC_CONTINUOUS
      , VtxMtrlCombos (&vmc)[MAX_HM_RES][MAX_HM_RES]
   #endif
      ) : height(height), mtrl_index(mtrl_index)
   #if !VMC_CONTINUOUS
      , vmc(vmc)
   #endif
   {
      Zero(material_blends); // initialize 'material_blends' to zeros which is required by 'processMaterials'
      lod_parts=null;
      cur_mtrl_index.set(255, 255, 255, 255); // !! set all to 255 so it will be always different than tested materials, since the actual values will never be set to (255,255,255,255) since the same material will get set only in the first channel (255,0,0,0), and in comparisons we don't need to check "cur_mtrl_combo<0 || cur_mtrl_index!=mtrl_index" but just "cur_mtrl_index!=mtrl_index"
      cur_mtrl_combo=-1;
   }
   void clear(Int res)
   {
   #if VMC_CONTINUOUS
      vmc_elms=0;
      REPD(y, res)
      REPD(x, res)vmc_first[y][x]=~0;
   #else
      REPD(y, res)
      REPD(x, res)vmc[y][x].clear();
   #endif
      REPAO(mtrl_combos).reset();
   }

   INLINE VecB4& mtrlIndex(Int x, Int y)
   {
      return mtrl_index.pixB4(x, y);
   }
   Int getMtrlComboIndex(C VecB4 &mtrl_index) // get index of material combination for given material indexes, 'mtrl_index' must be sorted (having components from biggest value to smallest), this function should be multi-threaded safe
   {
      // first find existing
      REPA(mtrl_combos)if(mtrl_combos[i].mtrl_index==mtrl_index)return i;

      // add new one
      MtrlCombo &mtrl_combo=mtrl_combos.New();
                 mtrl_combo.mtrl_index=mtrl_index;
                 mtrl_combo.reset();
      return     mtrl_combos.elms()-1;
   }
   INLINE void processMaterials(VecB4 &mtrl_index, Byte *mi, MB1 *mb, Int mtrls)
   {
      // set materials blend
      REP(mtrls)material_blends[mi[i]]+=mb[i]; // 'material_blends' is assumed to be set to zeros on function start

      // get materials with highest blend
      MBNL mtrl_blend=0;
      REP(mtrls)
      {
            Byte j=mi[i];
         if(MB1L b=material_blends[j])
         {
            material_blends[j]=0; // restore 'material_blends' to initial zeros, this will also prevent the same material from being processed again in this loop
            if(j) // don't add null materials (this could happen in case null material had non-zero blend set)
            {
               // add to 'mtrl_index' and 'mtrl_blend' while preserving order of most important materials
               FREPA(mtrl_blend)if(b>mtrl_blend.c[i]) // check if this material has higher blend than any listed
               {
                  for(Int last=Elms(mtrl_index)-1; last>i; last--) // move others to the right ('last' is the index of last material)
                  {
                     mtrl_index.c[last]=mtrl_index.c[last-1];
                     mtrl_blend.c[last]=mtrl_blend.c[last-1];
                  }
                  // set this material to the i-th slot
                  mtrl_index.c[i]=j;
                  mtrl_blend.c[i]=b;
                  break;
               }
            }
         }
      }

      // remove minor materials
      if(mtrl_index.c[1]){if(mtrl_blend.c[1]<=mtrl_blend.x        *MATERIAL2_EPS)mtrl_index.c[1]=mtrl_index.c[2]=mtrl_index.c[3]=0;else // if #1 is much smaller than #0      , then remove #1,#2,#3
      if(mtrl_index.c[2]){if(mtrl_blend.c[2]<=mtrl_blend.xy .sum()*MATERIAL3_EPS)                mtrl_index.c[2]=mtrl_index.c[3]=0;else // if #2 is much smaller than #0+#1   , then remove    #2,#3
      if(mtrl_index.c[3]){if(mtrl_blend.c[3]<=mtrl_blend.xyz.sum()*MATERIAL4_EPS)                                mtrl_index.c[3]=0;}}}  // if #3 is much smaller than #0+#1+#2, then remove       #3

      // sort by indexes (required by 'getMtrlComboIndex')
      if(mtrl_index.c[1])
      {
         if(mtrl_index.c[2])
         {
            if(mtrl_index.c[3])
            {
               if(mtrl_index.c[3]>mtrl_index.c[0])Swap(mtrl_index.c[3], mtrl_index.c[0]);
               if(mtrl_index.c[3]>mtrl_index.c[1])Swap(mtrl_index.c[3], mtrl_index.c[1]);
               if(mtrl_index.c[3]>mtrl_index.c[2])Swap(mtrl_index.c[3], mtrl_index.c[2]);
            }
            if(mtrl_index.c[2]>mtrl_index.c[0])Swap(mtrl_index.c[2], mtrl_index.c[0]);
            if(mtrl_index.c[2]>mtrl_index.c[1])Swap(mtrl_index.c[2], mtrl_index.c[1]);
         }
         if(mtrl_index.c[1]>mtrl_index.c[0])Swap(mtrl_index.c[1], mtrl_index.c[0]);
      }
   }
#if VMC_CONTINUOUS
   void setUsedByMtrlCombo(Int x, Int y, Int mtrl_combo, Memt<MtrlCombo> &mtrl_combos)
   {
      UInt i=vmc_first[y][x];
      if(  i==~0) // not yet added
      {
         RANGE_ASSERT(vmc_elms, vmc);
         VtxMtrlCombo &v=vmc[vmc_elms]; vmc_first[y][x]=vmc_elms++;
         v.next_add    =0;
         v.mtrl_combo  =mtrl_combo;
         v.mc_vtx_index=mtrl_combos[mtrl_combo].vtxs++;
      }else
      for(;;)
      {
         VtxMtrlCombo &c=vmc[i];
         if(c.mtrl_combo==mtrl_combo)return; // already listed by this mtrl combo
         if(c.next_add)i+=c.next_add;else    // proceed to the next one
         { // add new one
            RANGE_ASSERT(vmc_elms, vmc);
            c.next_add=vmc_elms-i;
            VtxMtrlCombo &v=vmc[vmc_elms++];
            v.next_add    =0;
            v.mtrl_combo  =mtrl_combo;
            v.mc_vtx_index=mtrl_combos[mtrl_combo].vtxs++;
         }
      }
   }
   Int vtxIndexInMtrlCombo(Int x, Int y, Int mtrl_combo)
   {
      for(UInt i=vmc_first[y][x]; ; )
      {
         RANGE_ASSERT(i, vmc_elms);
         VtxMtrlCombo &v=vmc[i];
         if(v.mtrl_combo==mtrl_combo)return v.mc_vtx_index;
         DEBUG_ASSERT(v.next_add!=0, "vtxIndexInMtrlCombo");
         i+=v.next_add;
      }
   }
#else
   INLINE void setUsedByMtrlCombo(Int x, Int y, Int mtrl_combo, Memt<MtrlCombo> &mtrl_combos) {       vmc[y][x]. setUsedByMtrlCombo(mtrl_combo, mtrl_combos);}
   INLINE Int vtxIndexInMtrlCombo(Int x, Int y, Int mtrl_combo                              ) {return vmc[y][x].vtxIndexInMtrlCombo(mtrl_combo);}
#endif

   Int getMtrlComboForTri(C VecI2 &p0, C VecI2 &p1, C VecI2 &p2)
   {
      VecB4 mtrl_index=0;

      // get nearby vertexes
      VecB4 mi[3];
      MBN   mb[3];
      if((mi[0]=mtrlIndex(p0.x, p0.y)).any() // get material indexes
      && (mi[1]=mtrlIndex(p1.x, p1.y)).any()
      && (mi[2]=mtrlIndex(p2.x, p2.y)).any())
      {
         mb[0]=mtrl_blend[p0.y][p0.x]; // get material blends
         mb[1]=mtrl_blend[p1.y][p1.x];
         mb[2]=mtrl_blend[p2.y][p2.x];

      #if __GNUC__
         processMaterials(mtrl_index, (Byte*)mi, (MB1*)mb, 3*4); // !! GCC may put the variables in the array in non-continuous memory, this cast fixes that !!
      #else
         processMaterials(mtrl_index, mi[0].c, mb[0].c, 3*4);
      #endif
      }

      // find element with current materials
      if(/*cur_mtrl_combo<0 || */cur_mtrl_index!=mtrl_index) // new element is needed
      {
         cur_mtrl_combo=getMtrlComboIndex(mtrl_index);
         cur_mtrl_index=mtrl_index;
      }

      setUsedByMtrlCombo(p0.x, p0.y, cur_mtrl_combo, mtrl_combos);
      setUsedByMtrlCombo(p1.x, p1.y, cur_mtrl_combo, mtrl_combos);
      setUsedByMtrlCombo(p2.x, p2.y, cur_mtrl_combo, mtrl_combos);

      mtrl_combos[cur_mtrl_combo].tris++;
      return      cur_mtrl_combo;
   }
   Int getMtrlComboForQuad(Int x, Int y, Int x1, Int y1)
   {
      VecB4 mtrl_index=0;

      // get nearby vertexes
      VecB4 mi[4];
      MBN   mb[4];
      if((mi[0]=mtrlIndex(x , y1)).any() // get material indexes
      && (mi[1]=mtrlIndex(x1, y1)).any()
      && (mi[2]=mtrlIndex(x1, y )).any()
      && (mi[3]=mtrlIndex(x , y )).any())
      {
         mb[0]=mtrl_blend[y1][x ]; // get material blends
         mb[1]=mtrl_blend[y1][x1];
         mb[2]=mtrl_blend[y ][x1];
         mb[3]=mtrl_blend[y ][x ];

      #if __GNUC__
         processMaterials(mtrl_index, (Byte*)mi, (MB1*)mb, 4*4); // !! GCC may put the variables in the array in non-continuous memory, this cast fixes that !!
      #else
         processMaterials(mtrl_index, mi[0].c, mb[0].c, 4*4);
      #endif
      }

      // find element with current materials
      if(/*cur_mtrl_combo<0 || */cur_mtrl_index!=mtrl_index) // new element is needed
      {
         cur_mtrl_combo=getMtrlComboIndex(mtrl_index);
         cur_mtrl_index=mtrl_index;
      }

      setUsedByMtrlCombo(x , y , cur_mtrl_combo, mtrl_combos);
      setUsedByMtrlCombo(x1, y , cur_mtrl_combo, mtrl_combos);
      setUsedByMtrlCombo(x , y1, cur_mtrl_combo, mtrl_combos);
      setUsedByMtrlCombo(x1, y1, cur_mtrl_combo, mtrl_combos);

      mtrl_combos[cur_mtrl_combo].tris+=2;
      return      cur_mtrl_combo;
   }

   void setTriFace(C VecI2 &p0, C VecI2 &p1, C VecI2 &p2, Int mtrl_combo_index)
   {
      MeshRender &mshr=(*lod_parts)[mtrl_combo_index].render;
      if(Byte *ind=(Byte*)mshr.indLockedData())
      {
         Int i0=vtxIndexInMtrlCombo(p0.x, p0.y, mtrl_combo_index),
             i1=vtxIndexInMtrlCombo(p1.x, p1.y, mtrl_combo_index),
             i2=vtxIndexInMtrlCombo(p2.x, p2.y, mtrl_combo_index);
         ind+=mshr.tris()*mshr.triIndSize();

         if(mshr._ib.bit16()){U16 *d=(U16*)ind; d[0]=i0; d[1]=i1; d[2]=i2;}
         else                {U32 *d=(U32*)ind; d[0]=i0; d[1]=i1; d[2]=i2;}

         mshr._tris++;
      }
   }
   void setQuadFace(Int x, Int y, Int x1, Int y1, Int mtrl_combo_index)
   {
      MeshRender &mshr=(*lod_parts)[mtrl_combo_index].render;
      if(Byte *ind=(Byte*)mshr.indLockedData())
      {
         Int lb=vtxIndexInMtrlCombo(x , y , mtrl_combo_index),
             lf=vtxIndexInMtrlCombo(x , y1, mtrl_combo_index),
             rb=vtxIndexInMtrlCombo(x1, y , mtrl_combo_index),
             rf=vtxIndexInMtrlCombo(x1, y1, mtrl_combo_index);
         ind+=mshr.tris()*mshr.triIndSize();

         Flt hlb=height.pixF(x , y ),
             hlf=height.pixF(x , y1),
             hrb=height.pixF(x1, y ),
             hrf=height.pixF(x1, y1);
         if(Abs(hlb-hrf)<Abs(hrb-hlf))
         {
            if(mshr._ib.bit16()){U16 *d=(U16*)ind; d[0]=lb; d[1]=lf; d[2]=rf; d[3]=rf; d[4]=rb; d[5]=lb;}
            else                {U32 *d=(U32*)ind; d[0]=lb; d[1]=lf; d[2]=rf; d[3]=rf; d[4]=rb; d[5]=lb;}
         }else
         {
            if(mshr._ib.bit16()){U16 *d=(U16*)ind; d[0]=lb; d[1]=lf; d[2]=rb; d[3]=rb; d[4]=lf; d[5]=rf;}
            else                {U32 *d=(U32*)ind; d[0]=lb; d[1]=lf; d[2]=rb; d[3]=rb; d[4]=lf; d[5]=rf;}
         }

         mshr._tris+=2;
      }
   }

   void setTriFaceSoft(C VecI2 &p0, C VecI2 &p1, C VecI2 &p2, Int mtrl_combo_index)
   {
      MeshBase &mshb=(*lod_parts)[mtrl_combo_index].base;
      if(VecI *ind=mshb.tri.ind()) // this can be null for null materials
         ind[mshb.tri._elms++].set(vtxIndexInMtrlCombo(p0.x, p0.y, mtrl_combo_index),
                                   vtxIndexInMtrlCombo(p1.x, p1.y, mtrl_combo_index),
                                   vtxIndexInMtrlCombo(p2.x, p2.y, mtrl_combo_index));
   }
   void setQuadFaceSoft(Int x, Int y, Int x1, Int y1, Int mtrl_combo_index)
   {
      MeshBase &mshb=(*lod_parts)[mtrl_combo_index].base;
      if(VecI *ind=mshb.tri.ind()) // this can be null for null materials
      {
         Int lb=vtxIndexInMtrlCombo(x , y , mtrl_combo_index),
             lf=vtxIndexInMtrlCombo(x , y1, mtrl_combo_index),
             rb=vtxIndexInMtrlCombo(x1, y , mtrl_combo_index),
             rf=vtxIndexInMtrlCombo(x1, y1, mtrl_combo_index);

         Int *d=ind[mshb.tri._elms].c; mshb.tri._elms+=2;

         Flt hlb=height.pixF(x , y ),
             hlf=height.pixF(x , y1),
             hrb=height.pixF(x1, y ),
             hrf=height.pixF(x1, y1);
         if(Abs(hlb-hrf)<Abs(hrb-hlf))
         {
            d[0]=lb; d[1]=lf; d[2]=rf; d[3]=rf; d[4]=rb; d[5]=lb;
         }else
         {
            d[0]=lb; d[1]=lf; d[2]=rb; d[3]=rb; d[4]=lf; d[5]=rf;
         }
      }
   }

   // use NOINLINE to avoid compilers inlining because of big stack usage
   NOINLINE void downSampleNormals(Int res, Int step, Int edge_step)
   {
   #if VTX_COMPRESS
      const Bool  storage_signed=D.meshStorageSigned();
      const Int   ofs=step/2, edge_ofs=(edge_step+1)/2, edge_soft_range=edge_ofs*3, res_step=res-step, res1=res-1; // use "(edge_step+1)/2" which gives same results as "Max(1, edge_step/2)" for pow2 but faster
      // first store results into 'temp', if we would store directly in 'vtx_nrm' then one softening would affect all others
            VecB4 temp[MAX_HM_RES/2-1][MAX_HM_RES/2], // !! since we don't use this for processing edges, then we can allocate 2 elements less "(MAX_HM_RES/2+1)-2" = "MAX_HM_RES/2-1", however for [y][x] X dimension use "MAX_HM_RES/2" to get Pow2 size and faster addressing
                  temp_edge[4][MAX_HM_RES]; // 4 because of 4 edges

      // process inner cells (without the edges)
      for(Int y=step, ty=0; y<res_step; y+=step, ty++) // !! if changed to include edges then adjust 'temp' size !!
      for(Int x=step, tx=0; x<res_step; x+=step, tx++) // !! if changed to include edges then adjust 'temp' size !!
      {
         VecI nrm=0; Int weight=0;
         for(Int sy=y-ofs, wy=0; wy<3; sy+=ofs, wy++)//if(InRange(sy, res)) !! not needed since we're not processing edges here !!
         for(Int sx=x-ofs, wx=0; wx<3; sx+=ofs, wx++)//if(InRange(sx, res)) !! not needed since we're not processing edges here !!
         {
          C Int    w=DownSizeWeights[wy][wx]; weight+=w;
          C VecB4 &n=        vtx_nrm[sy][sx];
            if(storage_signed)
            {
               nrm.x+=w*I8(n.x);
               nrm.y+=w*I8(n.y);
               nrm.z+=w*I8(n.z);
            }else
            {
               nrm.x+=w*(n.x-128);
               nrm.y+=w*(n.y-128);
               nrm.z+=w*(n.z-128);
            }
         }
         nrm/=weight; // !! assumes weight>0 !!
         VecB4 &n=temp[ty][tx];
         if(storage_signed)
         {
            n.x=nrm.x;
            n.y=nrm.y;
            n.z=nrm.z;
         }else
         {
            n.x=nrm.x+128;
            n.y=nrm.y+128;
            n.z=nrm.z+128;
         }
      }

      // process outer edges
      for(Int i=0; i<res; i+=edge_step) // march along main axis
      {
         VecI nrm[4]={0, 0, 0, 0}; Int weight[4]={0, 0, 0, 0};
         for(Int si=Max(i-edge_soft_range, 0), si_end=Min(res1, i+edge_soft_range); si<=si_end; si+=edge_ofs)
       //for(Int j=0; j<=ofs; j+=ofs) march along secondary axis, currently not implemented, to achieve edge normals having same values on neighbor areas, if we would average with the vtxs on the inside, then areas would have different edge normals, alternatively we could check for vtx normals of neighborhood areas, but that would require doing more calculations
         {
          C Int w=1;
            weight[0]+=w;
            weight[1]+=w;
            weight[2]+=w;
            weight[3]+=w;

          C VecB4 &n0=vtx_nrm[   0][si],
                  &n1=vtx_nrm[res1][si],
                  &n2=vtx_nrm[si][   0],
                  &n3=vtx_nrm[si][res1];

            if(storage_signed)
            {
               nrm[0].x+=w*I8(n0.x); nrm[0].y+=w*I8(n0.y); nrm[0].z+=w*I8(n0.z);
               nrm[1].x+=w*I8(n1.x); nrm[1].y+=w*I8(n1.y); nrm[1].z+=w*I8(n1.z);
               nrm[2].x+=w*I8(n2.x); nrm[2].y+=w*I8(n2.y); nrm[2].z+=w*I8(n2.z);
               nrm[3].x+=w*I8(n3.x); nrm[3].y+=w*I8(n3.y); nrm[3].z+=w*I8(n3.z);
            }else
            {
               nrm[0].x+=w*(n0.x-128); nrm[0].y+=w*(n0.y-128); nrm[0].z+=w*(n0.z-128);
               nrm[1].x+=w*(n1.x-128); nrm[1].y+=w*(n1.y-128); nrm[1].z+=w*(n1.z-128);
               nrm[2].x+=w*(n2.x-128); nrm[2].y+=w*(n2.y-128); nrm[2].z+=w*(n2.z-128);
               nrm[3].x+=w*(n3.x-128); nrm[3].y+=w*(n3.y-128); nrm[3].z+=w*(n3.z-128);
            }
         }
         REPD(e, 4)
         {
            VecI &edge_nrm=nrm[e];
            edge_nrm/=weight[e]; // !! assumes weight>0 !!
            VecB4 &n=temp_edge[e][i];
            if(storage_signed)
            {
               n.x=edge_nrm.x;
               n.y=edge_nrm.y;
               n.z=edge_nrm.z;
            }else
            {
               n.x=edge_nrm.x+128;
               n.y=edge_nrm.y+128;
               n.z=edge_nrm.z+128;
            }
         }
      }

      // store results
      for(Int y=step, ty=0; y<res_step; y+=step, ty++)
      for(Int x=step, tx=0; x<res_step; x+=step, tx++)vtx_nrm[y][x]=temp[ty][tx];

      for(Int i=0; i<res; i+=edge_step)
      {
         vtx_nrm[   0][i]=temp_edge[0][i];
         vtx_nrm[res1][i]=temp_edge[1][i];
         vtx_nrm[i][   0]=temp_edge[2][i];
         vtx_nrm[i][res1]=temp_edge[3][i];
      }
   #else
      TODO
   #endif
   }
   // use NOINLINE to avoid compilers inlining because of big stack usage
   NOINLINE void downSampleNormalsSoft(Int res, Int step, Int edge_step, Vec (&vtx_nrm)[MAX_HM_RES][MAX_HM_RES])
   {
      const Int ofs=step/2, edge_ofs=(edge_step+1)/2, edge_soft_range=edge_ofs*3, res_step=res-step, res1=res-1; // use "(edge_step+1)/2" which gives same results as "Max(1, edge_step/2)" for pow2 but faster
      // first store results into 'temp', if we would store directly in 'vtx_nrm' then one softening would affect all others
            Vec temp[MAX_HM_RES/2-1][MAX_HM_RES/2], // !! since we don't use this for processing edges, then we can allocate 2 elements less "(MAX_HM_RES/2+1)-2" = "MAX_HM_RES/2-1", however for [y][x] X dimension use "MAX_HM_RES/2" to get Pow2 size and faster addressing
                temp_edge[4][MAX_HM_RES]; // 4 because of 4 edges

      // process inner cells (without the edges)
      for(Int y=step, ty=0; y<res_step; y+=step, ty++) // !! if changed to include edges then adjust 'temp' size !!
      for(Int x=step, tx=0; x<res_step; x+=step, tx++) // !! if changed to include edges then adjust 'temp' size !!
      {
         Vec nrm=0; Int weight=0;
         for(Int sy=y-ofs, wy=0; wy<3; sy+=ofs, wy++)//if(InRange(sy, res)) !! not needed since we're not processing edges here !!
         for(Int sx=x-ofs, wx=0; wx<3; sx+=ofs, wx++)//if(InRange(sx, res)) !! not needed since we're not processing edges here !!
         {
          C Int  w=DownSizeWeights[wy][wx]; weight+=w;
          C Vec &n=        vtx_nrm[sy][sx];
            nrm.x+=w*n.x;
            nrm.y+=w*n.y;
            nrm.z+=w*n.z;
         }
         nrm/=weight; // !! assumes weight>0 !!
         temp[ty][tx]=nrm;
      }

      // process outer edges
      for(Int i=0; i<res; i+=edge_step) // march along main axis
      {
         Vec nrm[4]={0, 0, 0, 0}; Int weight[4]={0, 0, 0, 0};
         for(Int si=Max(i-edge_soft_range, 0), si_end=Min(res1, i+edge_soft_range); si<=si_end; si+=edge_ofs)
       //for(Int j=0; j<=ofs; j+=ofs) march along secondary axis, currently not implemented, to achieve edge normals having same values on neighbor areas, if we would average with the vtxs on the inside, then areas would have different edge normals, alternatively we could check for vtx normals of neighborhood areas, but that would require doing more calculations
         {
          C Int w=1;
            weight[0]+=w;
            weight[1]+=w;
            weight[2]+=w;
            weight[3]+=w;

          C Vec &n0=vtx_nrm[   0][si],
                &n1=vtx_nrm[res1][si],
                &n2=vtx_nrm[si][   0],
                &n3=vtx_nrm[si][res1];

            nrm[0].x+=w*n0.x; nrm[0].y+=w*n0.y; nrm[0].z+=w*n0.z;
            nrm[1].x+=w*n1.x; nrm[1].y+=w*n1.y; nrm[1].z+=w*n1.z;
            nrm[2].x+=w*n2.x; nrm[2].y+=w*n2.y; nrm[2].z+=w*n2.z;
            nrm[3].x+=w*n3.x; nrm[3].y+=w*n3.y; nrm[3].z+=w*n3.z;
         }
         REPD(e, 4)
         {
            Vec &edge_nrm=nrm[e];
            edge_nrm/=weight[e]; // !! assumes weight>0 !!
            temp_edge[e][i]=edge_nrm;
         }
      }

      // store results
      for(Int y=step, ty=0; y<res_step; y+=step, ty++)
      for(Int x=step, tx=0; x<res_step; x+=step, tx++)vtx_nrm[y][x]=temp[ty][tx];

      for(Int i=0; i<res; i+=edge_step)
      {
         vtx_nrm[   0][i]=temp_edge[0][i];
         vtx_nrm[res1][i]=temp_edge[1][i];
         vtx_nrm[i][   0]=temp_edge[2][i];
         vtx_nrm[i][res1]=temp_edge[3][i];
      }
   }
   void downSampleMaterials(Int res, Int step)
   {
   #if 0 // this doesn't improve the quality greatly, so let's just skip it
      Int ofs=step; step<<=1;
      for(Int y=0; y<res; y+=step)
      for(Int x=0; x<res; x+=step)
      {
         // set materials blend
         Byte mtrl_indexes[3*3*4]; Int mtrl_indexes_num=0;
         for(Int sy=y-ofs; sy<=y+ofs; sy+=ofs)if(InRange(sy, res))
         for(Int sx=x-ofs; sx<=x+ofs; sx+=ofs)if(InRange(sx, res))
         {
            VecB4 &mi=T.mtrl_index[sy][sx];
            Vec4  &mb=T.mtrl_blend[sy][sx]; this could be VecB4
            REPA(mi)
            {
               RANGE_ASSERT(mtrl_indexes_num, mtrl_indexes);
               mtrl_indexes[mtrl_indexes_num++]=mi.c[i];
               material_blends[mi.c[i]]+=mb.c[i];
            }
         }

         // get materials with highest blend
         VecB4 mtrl_index=0;
         Vec4  mtrl_blend=0;
         REP(  mtrl_indexes_num)
         {
               Byte j=mtrl_indexes[i];
            if(Flt  b=material_blends[j])
            {
               material_blends[j]=0; // restore 'material_blends' to initial zeros
               FREPA(mtrl_blend)if(b>mtrl_blend.c[i])
               {
                  for(Int last=Elms(mtrl_index); --last>i; )
                  {
                     mtrl_index.c[last]=mtrl_index.c[last-1];
                     mtrl_blend.c[last]=mtrl_blend.c[last-1];
                  }
                  mtrl_index.c[i]=j;
                  mtrl_blend.c[i]=b;
                  break;
               }
            }
         }

         // store
         T.mtrl_index[y][x]=mtrl_index;
         T.mtrl_blend[y][x]=mtrl_blend;
      }
   #endif
   }
};
/******************************************************************************/
// use NOINLINE to avoid compilers inlining because of big stack usage
NOINLINE Bool Heightmap::buildEx2(Mesh &mesh, Int quality, Flt tex_scale, UInt flag, BuildMemSoft /* !! Warning: this may be only 'BuildMem' !! */ &mem, C Heightmap *h_l, C Heightmap *h_r, C Heightmap *h_b, C Heightmap *h_f, C Heightmap *h_lb, C Heightmap *h_lf, C Heightmap *h_rb, C Heightmap *h_rf) // this function should be multi-threaded safe
{
   Int res=resolution();

   // validate params
   Clamp(quality, 0, 2);

   if(h_l  && h_l ->resolution()!=res)h_l =null;
   if(h_r  && h_r ->resolution()!=res)h_r =null;
   if(h_b  && h_b ->resolution()!=res)h_b =null;
   if(h_f  && h_f ->resolution()!=res)h_f =null;
   if(h_lb && h_lb->resolution()!=res)h_lb=null;
   if(h_lf && h_lf->resolution()!=res)h_lf=null;
   if(h_rb && h_rb->resolution()!=res)h_rb=null;
   if(h_rf && h_rf->resolution()!=res)h_rf=null;

 C Image *hl =(h_l  ? &h_l ->_height : null),
         *hr =(h_r  ? &h_r ->_height : null),
         *hb =(h_b  ? &h_b ->_height : null),
         *hf =(h_f  ? &h_f ->_height : null),
         *hlb=(h_lb ? &h_lb->_height : null),
         *hlf=(h_lf ? &h_lf->_height : null),
         *hrb=(h_rb ? &h_rb->_height : null),
         *hrf=(h_rf ? &h_rf->_height : null);

   // constants
   const Int  res1    =res-1,
              step    =(1<<quality),
              ao_step =3,
              ao_range=2*ao_step;
   const Flt  nrm_y   =2.00f/res1,
              ao_mul  =0.19f*res1;
   const Bool ambient_occlusion=FlagTest(flag, HM_AO),
              soft             =FlagTest(flag, HM_SOFT),
              build_null_mtrl  =FlagTest(flag, HM_BUILD_NULL_MTRL);

   // vars
   Flt     sharpness=0; Int sharpness_count=0;
   Builder builder(_height, _mtrl_index
#if !VMC_CONTINUOUS
   , mem.vmc
#endif
   );
   Flt min_y= FLT_MAX,
       max_y=-FLT_MAX;
   VecB4 (*const NrmToByte4)(C Vec &v)=(D.meshStorageSigned() ? NrmToSByte4 : NrmToUByte4);

   // set per-vertex data and calculate min/max height
   for(Int y=0; y<res; y+=step)
   {
      builder.frac[y]=y/Flt(res1);
      //Flt fy=y/Flt(res1);

      for(Int x=0; x<res; x+=step)
      {
         //Flt fx=x/Flt(res1);

         //builder.mtrl_index[y][x]=_mtrl_index.pixB4(x, y);
      #if MTRL_BLEND_HP
         if(_mtrl_blend.hwType()==IMAGE_F32_4)builder.mtrl_blend[y][x]=_mtrl_blend.pixF4(x, y);
         else                                 builder.mtrl_blend[y][x]=_mtrl_blend.pixB4(x, y); // doesn't need to be scaled if all blends have the same scale
      #else
         builder.mtrl_blend[y][x]=MtrlBlendB(_mtrl_blend, x, y);
      #endif

         Flt h=_height.pixF(x, y);

         // min/max height
         MIN(min_y, h);
         MAX(max_y, h);

         // normal
      #if VTX_COMPRESS
         Vec  nrm;
      #else
         Vec &nrm=builder.vtx_nrm[y][x];
      #endif
         Vec2 ddh;

         Flt l, r;
         if(x!=   0)l=_height.pixF(x-1, y);else if(hl)l=hl->pixF(res-2, y);else{nrm.x=(_height.pixF(    0, y)-_height.pixF(   1, y))*2; goto processed_x;}
         if(x!=res1)r=_height.pixF(x+1, y);else if(hr)r=hr->pixF(    1, y);else{nrm.x=(_height.pixF(res-2, y)-_height.pixF(res1, y))*2; goto processed_x;}
         nrm.x=l-r;
         sharpness+=Abs(h-Avg(l, r)); sharpness_count++;
      processed_x:

         Flt b, f;
         if(y!=   0)b=_height.pixF(x, y-1);else if(hb)b=hb->pixF(x, res-2);else{nrm.z=(_height.pixF(x,     0)-_height.pixF(x,    1))*2; goto processed_z;}
         if(y!=res1)f=_height.pixF(x, y+1);else if(hf)f=hf->pixF(x,     1);else{nrm.z=(_height.pixF(x, res-2)-_height.pixF(x, res1))*2; goto processed_z;}
         nrm.z=b-f;
         sharpness+=Abs(h-Avg(b, f)); sharpness_count++;
      processed_z:

         ddh=nrm.xz();

         nrm.y=nrm_y; nrm.normalize();
      #if VTX_COMPRESS
         if(soft) mem.vtx_nrm[y][x]=nrm;
         else builder.vtx_nrm[y][x]=NrmToByte4(nrm);
      #endif

         // pos, tex, tan
         //builder.vtx_pos[y][x].set(fx, h, fy);
      #if !VTX_HEIGHTMAP
         builder.vtx_tan[y][x]=!PointOnPlane(Vec(1, 0, 0), nrm);
         builder.vtx_tex[y][x].set(( fx+area._xz.x)*tex_scale,
                                   (-fy-area._xz.y)*tex_scale);
      #endif

         if(ambient_occlusion)
         {
            Flt occl=0, power=EPS; // EPS prevents division by zero
            ddh*=-0.5f;

            for(Int dy=-ao_range; dy<=ao_range; dy+=ao_step)
            for(Int dx=-ao_range; dx<=ao_range; dx+=ao_step)
            {
               Int tx=x+dx,
                   ty=y+dy;
               Flt th;
               if(tx<0)
               {
                  if(ty<   0){if(hlb)th=hlb->pixF(tx+res1, ty+res1);else th=h                  +(_height.pixF(0,    0)-_height.pixF(1,      1))*Dist(dx, dy);}else
                  if(ty>=res){if(hlf)th=hlf->pixF(tx+res1, ty-res1);else th=h                  +(_height.pixF(0, res1)-_height.pixF(1, res1-1))*Dist(dx, dy);}else
                             {if(hl )th=hl ->pixF(tx+res1, ty     );else th=_height.pixF(x, ty)-(_height.pixF(0,   ty)-_height.pixF(1,     ty))*dx;} // here 'dx' is negative
               }else
               if(tx>=res)
               {
                  if(ty<   0){if(hrb)th=hrb->pixF(tx-res1, ty+res1);else th=h                  +(_height.pixF(res1,    0)-_height.pixF(res1-1,      1))*Dist(dx, dy);}else
                  if(ty>=res){if(hrf)th=hrf->pixF(tx-res1, ty-res1);else th=h                  +(_height.pixF(res1, res1)-_height.pixF(res1-1, res1-1))*Dist(dx, dy);}else
                             {if(hr )th=hr ->pixF(tx-res1, ty     );else th=_height.pixF(x, ty)+(_height.pixF(res1,   ty)-_height.pixF(res1-1,     ty))*dx;} // here 'dx' is positive
               }else
               {
                  if(ty<   0){if(hb)th=hb->pixF(tx, ty+res1);else th=_height.pixF(tx, y)-(_height.pixF(tx,    0)-_height.pixF(tx,      1))*dy;}else // here 'dy' is negative
                  if(ty>=res){if(hf)th=hf->pixF(tx, ty-res1);else th=_height.pixF(tx, y)+(_height.pixF(tx, res1)-_height.pixF(tx, res1-1))*dy;}else // here 'dy' is positive
                                    th=_height.pixF(tx, ty);
               }
               Flt d=th - h - dx*ddh.x - dy*ddh.y,
                   o=Sat(d*ao_mul),
                   p=Lerp(0.3f, 1.0f, BlendSqr(Dist(dx, dy)/ao_range));
               occl +=p*o;
               power+=p;
            }
            builder.occlusion[y][x]=FltToByte(1-occl/power);
         }
      }
   }

#if 0
   mesh.box       .set(0, min_y, 0, 1, max_y, 1);
   mesh.lod_center.set(0.5f, mesh.box.centerY(), 0.5f);
#else
                   mesh.ext.ext.set(0.5f, (max_y-min_y)*0.5f, 0.5f);
   mesh.lod_center=mesh.ext.pos.set(0.5f, (min_y+max_y)*0.5f, 0.5f);
#endif

   // adjust sharpness
   AdjustSharpness(sharpness, sharpness_count);

   // downsample
   REP(quality)builder.downSampleMaterials(res, 1<<i);
   Image color; if(T._color.is()){T._color.copyTry(color); REP(quality)color.resize(color.w()/2+1, color.h()/2+1, FILTER_LINEAR, true, false, true);}

   // set LOD levels
   mesh.setLods(Max(1, Min(LODS, BitHi(Unsigned(res1)))-quality)); // BitHi(2)->1 (1 lod), BitHi(4)->2 (2 lods), BitHi(8)->3 (3 lods)
   FREPD(l, mesh.lods())
   {
      MeshLod &lod=mesh.lod(l); builder.lod_parts=&lod.parts;

      const Int L           =l+quality, // lod affected by quality
                edge_quality=Max(l-2, 0), // !! this affects number of elements in 'VtxMtrlCombos' !!
                LE          =L+edge_quality, // lod affected by quality and edge quality
                L_2         =(L ? 1<<(L-1) : 0),
                Step        =(step<<edge_quality); // step affected by edge quality
            Int from        =(   0>>L),
                to          =(res1>>L);
      if(l)
      {
         from++;
         to  --;

         // downsample
         if(soft)builder.downSampleNormalsSoft(res, step<<l, Step, mem.vtx_nrm); // normals
         else    builder.downSampleNormals    (res, step<<l, Step             ); // normals
                 builder.downSampleMaterials  (res, step<<(l-1)               ); // materials
         color.resize(color.w()/2+1, color.h()/2+1, FILTER_LINEAR, true, false, true); // color map
      }

      builder.clear(res);

      // for each face, test the index of Heightmap Element, and count the number of vtxs/tris/quads
      for(Int y=from; y<to; y++)
      for(Int x=from; x<to; x++)builder.map_mtrl_combo[y][x]=builder.getMtrlComboForQuad(x<<L, y<<L, (x+1)<<L, (y+1)<<L);

      // add edges
      if(l)
      {
         for(Int i=0; i<res1; i+=Step)
         {
            Int x=((i+L_2)>>L)<<L;
            if(x<=0   )x=     (1<<L);else
            if(x>=res1)x=res1-(1<<L);
            builder.edg_mtrl_combo[0][i]=builder.getMtrlComboForTri(VecI2(i+Step,    0), VecI2(i     ,    0), VecI2(x,       1<<L )); // back
            builder.edg_mtrl_combo[1][i]=builder.getMtrlComboForTri(VecI2(i     , res1), VecI2(i+Step, res1), VecI2(x, res1-(1<<L))); // forward
            builder.edg_mtrl_combo[2][i]=builder.getMtrlComboForTri(VecI2(0   , i     ), VecI2(0   , i+Step), VecI2(      1<<L , x)); // left
            builder.edg_mtrl_combo[3][i]=builder.getMtrlComboForTri(VecI2(res1, i+Step), VecI2(res1, i     ), VecI2(res1-(1<<L), x)); // right
         }
         for(Int i=from; i<to; i++)
         {
            Int e=((i+i+1)<<(L-1));
            builder.tri_mtrl_combo[0][i]=builder.getMtrlComboForTri(VecI2( i   <<L,       1<<L ), VecI2((i+1)<<L,       1<<L ), VecI2(e,    0)); // back
            builder.tri_mtrl_combo[1][i]=builder.getMtrlComboForTri(VecI2((i+1)<<L, res1-(1<<L)), VecI2( i   <<L, res1-(1<<L)), VecI2(e, res1)); // forward
            builder.tri_mtrl_combo[2][i]=builder.getMtrlComboForTri(VecI2(      1<<L , (i+1)<<L), VecI2(      1<<L ,  i   <<L), VecI2(   0, e)); // left
            builder.tri_mtrl_combo[3][i]=builder.getMtrlComboForTri(VecI2(res1-(1<<L),  i   <<L), VecI2(res1-(1<<L), (i+1)<<L), VecI2(res1, e)); // right
         }
      }

      // create mesh lods
      if(lod.parts.elms()!=builder.mtrl_combos.elms())lod.create(builder.mtrl_combos.elms());
      lod.dist(GetLodDist(l, sharpness));

      // create mesh parts
      FREPA(builder.mtrl_combos)
      {
         MtrlCombo &mtrl_combo=builder.mtrl_combos[i];
         MeshPart  &part      =    lod.parts      [i];

         UInt flag=VTX_POS|VTX_NRM|TRI_IND;
         if(!VTX_HEIGHTMAP)flag|=VTX_TEX0|VTX_TAN;
         if(ambient_occlusion || color.is())flag|=VTX_COLOR;
         if( mtrl_combo.mtrl_index.y/* || mtrl_combo.mtrl_index.z || mtrl_combo.mtrl_index.w*/)flag|=VTX_MATERIAL;else // 'mtrl_index' in 'mtrl_combo' is always sorted, so we only need to check if 2nd material is not null
         if(!mtrl_combo.mtrl_index.x && !build_null_mtrl)mtrl_combo.reset(); // prevent creating null material

         if(soft)
         {
            MeshBase &mshb=part.base;
            mshb.create(mtrl_combo.vtxs, 0, mtrl_combo.tris, 0, flag);
            mshb.tri._elms=0; // clear so we can use it as progress index for adding new tris
         }else
         {
            MeshRender &mshr=part.render;
            if(!mshr.create(mtrl_combo.vtxs, mtrl_combo.tris, flag, VTX_COMPRESS))return false;

            if(mshr.vtxs() && !mshr.vtxLock(LOCK_WRITE))return false;
            if(mshr.tris() && !mshr.indLock(LOCK_WRITE))return false;
            mshr._tris=0; // clear so we can use it as progress index for adding new tris

            mtrl_combo.ofs_pos     =mshr.vtxOfs(VTX_POS     );
            mtrl_combo.ofs_nrm     =mshr.vtxOfs(VTX_NRM     );
            mtrl_combo.ofs_color   =mshr.vtxOfs(VTX_COLOR   );
            mtrl_combo.ofs_material=mshr.vtxOfs(VTX_MATERIAL);
         }
      }

      // store in mesh
      if(soft)
      {
         // set vertexes
         REPD(y, res)
         REPD(x, res)
         {
         #if VMC_CONTINUOUS
            UInt i=builder.vmc_first[y][x]; if(i!=~0)
            for(;;)
            {
               VtxMtrlCombo &vmc=builder.vmc[i]; Int mc=vmc.mtrl_combo;
         #else
            VtxMtrlCombos &vmc_xy=mem.vmc[y][x]; FREPA(vmc_xy.vmc)
            {
               VtxMtrlCombo &vmc       =   vmc_xy.vmc        [ i]; Int mc=vmc.mtrl_combo; if(mc==VMC_NULL)break;
         #endif
               MtrlCombo    &mtrl_combo=  builder.mtrl_combos[mc];
               MeshBase     &mshb      =(*builder.lod_parts )[mc].base;
               if(mshb.vtx._elms) // this can be 0 for null materials
               {
                  Int vtx_index=vmc.mc_vtx_index;

                  mshb.vtx.pos(vtx_index).set(builder.frac[x], _height.pixF(x, y), builder.frac[y]); // position
                  mshb.vtx.nrm(vtx_index)=mem.vtx_nrm[y][x]; // normal
                  if(mshb.vtx.material()) // material
                  {
                     VecB4 &vtx_mtrl=mshb.vtx.material(vtx_index);
                   C VecB4 & mc_mtrl=mtrl_combo.mtrl_index,
                           & hm_mtrl=          _mtrl_index.pixB4(x, y);
                  #if 1
                     vtx_mtrl=VtxMtrlBlend(mc_mtrl, hm_mtrl, builder.mtrl_blend[y][x]);
                  #else
                     if(_mtrl_blend.hwType()==IMAGE_F32_4)vtx_mtrl=VtxMtrlBlend(mc_mtrl, hm_mtrl, _mtrl_blend.pixF4(x, y));
                     else                                 vtx_mtrl=VtxMtrlBlend(mc_mtrl, hm_mtrl, _mtrl_blend.pixB4(x, y));
                  #endif
                  }
                  if(mshb.vtx.color()) // color
                  {
                     Color &vtx_color=mshb.vtx.color(vtx_index);
                     if(ambient_occlusion)
                     {
                        Byte occl=builder.occlusion[y][x];
                        if(color.is())vtx_color=ColorBrightnessB(color.color(x>>L, y>>L), occl);
                        else          vtx_color.set(occl);
                     }else            vtx_color=color.color(x>>L, y>>L); // in this case 'Image color' always exists, because "mtrl_combo.ofs_color>=0" implies that vertex contains color, and it was created only if 'ambient_occlusion' or 'Image color' exists
                  }
               }
            #if VMC_CONTINUOUS
               if(vmc.next_add)i+=vmc.next_add;else break;
            #endif
            }
         }

         // set faces
         {
            // quads
            for(Int y=from; y<to; y++)
            for(Int x=from; x<to; x++)builder.setQuadFaceSoft(x<<L, y<<L, (x+1)<<L, (y+1)<<L, builder.map_mtrl_combo[y][x]);

            // tri (edge-skirts)
            if(l)
            {
               for(Int i=0; i<res1; i+=Step)
               {
                  Int x=((i+L_2)>>L)<<L;
                  if(x<=0   )x=     (1<<L);else
                  if(x>=res1)x=res1-(1<<L);
                  builder.setTriFaceSoft(VecI2(i+Step,    0), VecI2(i     ,    0), VecI2(x,      (1<<L)), builder.edg_mtrl_combo[0][i]); // back
                  builder.setTriFaceSoft(VecI2(i     , res1), VecI2(i+Step, res1), VecI2(x, res1-(1<<L)), builder.edg_mtrl_combo[1][i]); // forward
                  builder.setTriFaceSoft(VecI2(0   , i     ), VecI2(0   , i+Step), VecI2(      1<<L , x), builder.edg_mtrl_combo[2][i]); // left
                  builder.setTriFaceSoft(VecI2(res1, i+Step), VecI2(res1, i     ), VecI2(res1-(1<<L), x), builder.edg_mtrl_combo[3][i]); // right
               }
               for(Int i=from; i<to; i++)
               {
                  Int e=((i+i+1)<<(L-1));
                  builder.setTriFaceSoft(VecI2( i   <<L,       1<<L ), VecI2((i+1)<<L,       1<<L ), VecI2(e,    0), builder.tri_mtrl_combo[0][i]); // back
                  builder.setTriFaceSoft(VecI2((i+1)<<L, res1-(1<<L)), VecI2( i   <<L, res1-(1<<L)), VecI2(e, res1), builder.tri_mtrl_combo[1][i]); // forward
                  builder.setTriFaceSoft(VecI2(      1<<L , (i+1)<<L), VecI2(      1<<L ,  i   <<L), VecI2(   0, e), builder.tri_mtrl_combo[2][i]); // left
                  builder.setTriFaceSoft(VecI2(res1-(1<<L),  i   <<L), VecI2(res1-(1<<L), (i+1)<<L), VecI2(res1, e), builder.tri_mtrl_combo[3][i]); // right
               }
            }
         }
      }else
      {
         // set vertexes
         REPD(y, res)
         REPD(x, res)
         {
         #if VMC_CONTINUOUS
            UInt i=builder.vmc_first[y][x]; if(i!=~0)
            for(;;)
            {
               VtxMtrlCombo &vmc=builder.vmc[i]; Int mc=vmc.mtrl_combo;
         #else
            VtxMtrlCombos &vmc_xy=mem.vmc[y][x]; FREPA(vmc_xy.vmc)
            {
               VtxMtrlCombo &vmc       =   vmc_xy.vmc        [ i]; Int mc=vmc.mtrl_combo; if(mc==VMC_NULL)break;
         #endif
               MtrlCombo    &mtrl_combo=  builder.mtrl_combos[mc];
               MeshRender   &mshr      =(*builder.lod_parts )[mc].render;
               if(mshr.vtxLockedData())
               {
                  Byte *v=(Byte*)mshr.vtxLockedData()+vmc.mc_vtx_index*mshr.vtxSize();

                  ((Vec*)(v+mtrl_combo.ofs_pos))->set(builder.frac[x], _height.pixF(x, y), builder.frac[y]); // position

               #if VTX_COMPRESS
                  *((VecB4*)(v+mtrl_combo.ofs_nrm))=builder.vtx_nrm[y][x]; // normal
               #else
                  *((Vec  *)(v+mtrl_combo.ofs_nrm))=builder.vtx_nrm[y][x]; // normal
               #endif

                  if(mtrl_combo.ofs_material>=0) // material
                  {
                     VecB4 &vtx_mtrl=*((VecB4*)(v+mtrl_combo.ofs_material));
                   C VecB4 & mc_mtrl=mtrl_combo.mtrl_index,
                           & hm_mtrl=          _mtrl_index.pixB4(x, y);
                  #if 1
                     vtx_mtrl=VtxMtrlBlend(mc_mtrl, hm_mtrl, builder.mtrl_blend[y][x]);
                  #else
                     if(_mtrl_blend.hwType()==IMAGE_F32_4)vtx_mtrl=VtxMtrlBlend(mc_mtrl, hm_mtrl, _mtrl_blend.pixF4(x, y));
                     else                                 vtx_mtrl=VtxMtrlBlend(mc_mtrl, hm_mtrl, _mtrl_blend.pixB4(x, y));
                  #endif
                  }
                  if(mtrl_combo.ofs_color>=0) // color
                  {
                     Color &vtx_color=*((Color*)(v+mtrl_combo.ofs_color));
                     if(ambient_occlusion)
                     {
                        Byte occl=builder.occlusion[y][x];
                        if(color.is())vtx_color=ColorBrightnessB(color.color(x>>L, y>>L), occl);
                        else          vtx_color.set(occl);
                     }else            vtx_color=color.color(x>>L, y>>L); // in this case 'Image color' always exists, because "mtrl_combo.ofs_color>=0" implies that vertex contains color, and it was created only if 'ambient_occlusion' or 'Image color' exists
                  }
               }
            #if VMC_CONTINUOUS
               if(vmc.next_add)i+=vmc.next_add;else break;
            #endif
            }
         }

         // set faces
         {
            // quads
            for(Int y=from; y<to; y++)
            for(Int x=from; x<to; x++)builder.setQuadFace(x<<L, y<<L, (x+1)<<L, (y+1)<<L, builder.map_mtrl_combo[y][x]);

            // tri (edge-skirts)
            if(l)
            {
               for(Int i=0; i<res1; i+=Step)
               {
                  Int x=((i+L_2)>>L)<<L;
                  if(x<=0   )x=     (1<<L);else
                  if(x>=res1)x=res1-(1<<L);
                  builder.setTriFace(VecI2(i+Step,    0), VecI2(i     ,    0), VecI2(x,      (1<<L)), builder.edg_mtrl_combo[0][i]); // back
                  builder.setTriFace(VecI2(i     , res1), VecI2(i+Step, res1), VecI2(x, res1-(1<<L)), builder.edg_mtrl_combo[1][i]); // forward
                  builder.setTriFace(VecI2(0   , i     ), VecI2(0   , i+Step), VecI2(      1<<L , x), builder.edg_mtrl_combo[2][i]); // left
                  builder.setTriFace(VecI2(res1, i+Step), VecI2(res1, i     ), VecI2(res1-(1<<L), x), builder.edg_mtrl_combo[3][i]); // right
               }
               for(Int i=from; i<to; i++)
               {
                  Int e=((i+i+1)<<(L-1));
                  builder.setTriFace(VecI2( i   <<L,       1<<L ), VecI2((i+1)<<L,       1<<L ), VecI2(e,    0), builder.tri_mtrl_combo[0][i]); // back
                  builder.setTriFace(VecI2((i+1)<<L, res1-(1<<L)), VecI2( i   <<L, res1-(1<<L)), VecI2(e, res1), builder.tri_mtrl_combo[1][i]); // forward
                  builder.setTriFace(VecI2(      1<<L , (i+1)<<L), VecI2(      1<<L ,  i   <<L), VecI2(   0, e), builder.tri_mtrl_combo[2][i]); // left
                  builder.setTriFace(VecI2(res1-(1<<L),  i   <<L), VecI2(res1-(1<<L), (i+1)<<L), VecI2(res1, e), builder.tri_mtrl_combo[3][i]); // right
               }
            }
         }
      }

      // finalize mesh parts
      REPA(builder.mtrl_combos) // go from the end to delete
      {
         MeshPart &part=lod.parts[i];
         // first check if empty, then remove, this can happen for example if material combo was created for main LOD, but a smaller LOD didn't have this, or for null material materials with "build_null_mtrl==false"
         if(soft)
         {
            MeshBase &base=part.base;
            if(!base.is())goto remove_part;
         }else
         {
            MeshRender &mshr=part.render;
            if(!mshr.is())goto remove_part;
            mshr.indUnlock();
            mshr.vtxUnlock();
         }
         {
            MtrlCombo &mtrl_combo=builder.mtrl_combos[i];
            VecB4     &mtrl_index=mtrl_combo.mtrl_index;
         #if VTX_HEIGHTMAP
            part._vtx_heightmap=tex_scale; // !! set before calling 'multiMaterial' because this value affects the shader !!
         #endif
            part.multiMaterial(_materials[mtrl_index.c[0]], _materials[mtrl_index.c[1]], _materials[mtrl_index.c[2]], _materials[mtrl_index.c[3]], soft ? -1 : l); // set shaders only for hardware mode
         }

         continue; // continue without removing this part

      remove_part:
         lod.parts.remove(i, true);
      }
   }
   return true;
}
Bool Heightmap::buildEx(Mesh &mesh, Int quality, Flt tex_scale, UInt flag, BuildMemSoft &mem, C Heightmap *h_l, C Heightmap *h_r, C Heightmap *h_b, C Heightmap *h_f, C Heightmap *h_lb, C Heightmap *h_lf, C Heightmap *h_rb, C Heightmap *h_rf) // this function should be multi-threaded safe
{
   if(is())
   {
      // buildEx2 was separated because it uses a very big stack
      if(buildEx2(mesh, quality, tex_scale, flag, mem, h_l, h_r, h_b, h_f, h_lb, h_lf, h_rb, h_rf))
      {
         // remove empty lods
         REPD(l, mesh.lods())if(l && !mesh.lod(l).is())mesh.removeLod(l);

         mesh.sortByMaterials();
         return true;
      }
   }
   mesh.del(); return false;
}
/******************************************************************************/
void Heightmap::build(Mesh &dest_mesh, Int quality, Flt tex_scale, UInt flag, C Heightmap *l, C Heightmap *r, C Heightmap *b, C Heightmap *f, C Heightmap *lb, C Heightmap *lf, C Heightmap *rb, C Heightmap *rf, Mems<Byte> *temp_mem)
{
   if(is())
   {
      // !! prefer 'Mems' because 'Memc' for example will round up memory size to nearest Pow2
      Int build_mem_size=((flag&HM_SOFT) ? SIZE(BuildMemSoft) : SIZE(BuildMem));
      Ptr build_mem; if(temp_mem)build_mem=temp_mem->setNum(build_mem_size).data();else build_mem=Alloc(build_mem_size);
      buildEx(dest_mesh, quality, tex_scale, flag, *(BuildMemSoft*)build_mem, l, r, b, f, lb, lf, rb, rf);
      if(!temp_mem)Free(build_mem);
   }else dest_mesh.del();
}
/******************************************************************************/
void Heightmap::resize(Int res)
{
   Clamp(res, 2, MAX_HM_RES); res=NearestPow2(res)|1;
   if(res!=resolution())
   {
     _height    .resize(res, res, FILTER_BEST, true, false, true);
     _color     .resize(res, res, FILTER_BEST, true, false, true);
     _mtrl_index.resize(res, res, FILTER_NONE, true, false, true);
     _mtrl_blend.resize(res, res, FILTER_NONE, true, false, true);
   }
}
/******************************************************************************/
// IO
/******************************************************************************/
static Bool SaveAs(C Image &image, File &f, IMAGE_TYPE type)
{
   if(!image.is() || image.type()==type)return image.saveData(f);
   Image temp; if(image.copyTry(temp, -1, -1, -1, type))return temp.saveData(f);
   return false;
}
static Bool LoadAs(Image &image, File &f, IMAGE_TYPE type)
{
   if(!image.loadData(f))return false;
   if( image.is() && image.type()!=type)if(!image.copyTry(image, -1, -1, -1, type))return false;
   return true;
}
static Bool AddEmptyAlpha(Image &image)
{
   if(image.copyTry(image, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))if(image.lock())
   {
      REPD(y, image.lh())
      REPD(x, image.lw())image.pixC(x, y).a=0;
      image.unlock();
      return true;
   }
   return false;
}
Bool Heightmap::save(File &f, CChar *path)C
{
   f.cmpUIntV(2); // version
   if(       _height    .saveData(f        ))
   if(SaveAs(_color     , f, IMAGE_R8G8B8  ))
   if(       _mtrl_index.saveData(f        ))
   if(SaveAs(_mtrl_blend, f, IMAGE_R8G8B8A8))
   if(_materials.save(f, path))
      return f.ok();
   return false;
}
Bool Heightmap::load(File &f, CChar *path)
{
   del();

   switch(f.decUIntV()) // version
   {
      case 2:
      {
         if(_height    .loadData(f))
         if(_color     .loadData(f))
         if(_mtrl_index.loadData(f))
         if(_mtrl_blend.loadData(f))
         if(_materials .load    (f, path))
            if(f.ok())return true;
      }break;

      case 1:
      {
         if(       _height    .loadData(f)      )
         if(LoadAs(_color     , f, IMAGE_R8G8B8))
         if(LoadAs(_mtrl_index, f, IMAGE_R8G8B8))if(AddEmptyAlpha(_mtrl_index))
         if(LoadAs(_mtrl_blend, f, IMAGE_R8G8B8))if(AddEmptyAlpha(_mtrl_blend))
         if(_materials.load(f, path))
            if(f.ok())return true;
      }break;

      case 0:
      {
         if(       _height    .loadData(f)      )
         if(LoadAs(_color     , f, IMAGE_R8G8B8))
         if(LoadAs(_mtrl_index, f, IMAGE_B8G8R8))
         if(LoadAs(_mtrl_blend, f, IMAGE_R8G8B8))
         {
            if(_mtrl_index.hwType()==IMAGE_B8G8R8)_mtrl_index._type=_mtrl_index._hw_type=IMAGE_R8G8B8; // old version used 'IMAGE_B8G8R8' and the index for the first material was stored in first byte which is blue
            if(_materials.loadOld(f, path))
            {
               // check if any material index addresses non-existing material (this for some reason happened on one machine)
               REPD(y, _mtrl_index.h())
               REPD(x, _mtrl_index.w())
               {
                  VecB &v=_mtrl_index.pixB3(x, y);
                  {
                     if(v.x>=_materials.elms())v.x=0;
                     if(v.y>=_materials.elms())v.y=0;
                     if(v.z>=_materials.elms())v.z=0;
                  }
               }
               if(f.ok())
               if(AddEmptyAlpha(_mtrl_index))
               if(AddEmptyAlpha(_mtrl_blend))
                  return true;
            }
         }
      }break;
   }
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
