/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
WaterClass   Water;
WaterMtrl   *WaterMtrlLast;
WaterMtrlPtr WaterMtrlNull;
DEFINE_CACHE(WaterMtrl, WaterMtrls, WaterMtrlPtr, "Water Material");
/******************************************************************************/
// WATER PARAMETERS
/******************************************************************************/
WaterMtrl::WaterMtrl()
{
   density               =0.3f;
   density_add           =0.45f;
   density_underwater    =0.02f;
   density_underwater_add=0.6f;
   scale_color           =200;
   scale_normal          =10;
   scale_bump            =100;
   rough                 =1;
   reflect_tex           =0.1f;
   reflect_world         =0.18f;
   refract               =0.10f;
   refract_reflection    =0.06f;
   refract_underwater    =0.01f;
   specular              =1.5f;
   wave_scale            =0.25f;
   fresnel_pow           =5.5f;
   fresnel_rough         =4;
   fresnel_color    .set(0.10f, 0.10f, 0.10f);
   color            .set(0.42f, 0.50f, 0.58f);
   color_underwater0.set(0.26f, 0.35f, 0.42f);
   color_underwater1.set(0.10f, 0.20f, 0.30f);
}
WaterMtrl& WaterMtrl::    normalMap(C ImagePtr &image) {T.    _normal_map=image; return T;}
WaterMtrl& WaterMtrl::reflectionMap(C ImagePtr &image) {T._reflection_map=image; return T;}
WaterMtrl& WaterMtrl::     colorMap(C ImagePtr &image)
{
   T._color_map=image;

  _bump=null;
   if(image && image->is())
   {
   #if DX9
      if(D.validUsage(D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, image->hwType())) // if can use existing format
      {
   #endif
        _bump=image;
   #if DX9
      }else
      if(this==&Water) // check if can convert to supported format (this can be donly only for 'Water')
      {
         IMAGE_TYPE type=IMAGE_NONE;
         if(D.validUsage(D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, IMAGE_L8 ))type=IMAGE_L8 ;else
         if(D.validUsage(D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, IMAGE_A8 ))type=IMAGE_A8 ;else
         if(D.validUsage(D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, IMAGE_F32))type=IMAGE_F32;
         if(type)
         {
            Image temp, *src=image(); if(src->compressed())if(src->copyTry(temp, -1, -1, -1, IMAGE_A8, IMAGE_SOFT, 1))src=&temp;
            if(!src->compressed() && Water._bump_temp.createTry(src->w(), src->h(), 1, type, IMAGE_2D, 1, false))
            if( src->lock(LOCK_READ))
            {
               if(Water._bump_temp.lock(LOCK_WRITE))
               {
                  if(Water._bump_temp.hwType()==IMAGE_L8 || Water._bump_temp.hwType()==IMAGE_A8)
                  {
                     REPD(y, Water._bump_temp.h())
                     REPD(x, Water._bump_temp.w())Water._bump_temp.pixB(x, y)=src->color(x, y).a;
                  }else
                  if(Water._bump_temp.hwType()==IMAGE_F32)
                  {
                     REPD(y, Water._bump_temp.h())
                     REPD(x, Water._bump_temp.w())Water._bump_temp.pixF(x, y)=src->colorF(x, y).w;
                  }
                 _bump=&Water._bump_temp.unlock().updateMipMaps(FILTER_BEST, false);
               }
               src->unlock();
            }
         }
      }
   #endif
   }
   if(!_bump || _bump==image)Water._bump_temp.del();
   return T;
}
WaterMtrl& WaterMtrl::reset()
{
   T=WaterMtrl();
   return validate();
}
WaterMtrl& WaterMtrl::validate()
{
   if(WaterMtrlLast==this)WaterMtrlLast=null;
   return T;
}
void WaterMtrl::set()
{
   // Col[1], Col[2], Det[0] reserved for 'setImages'
   Sh.h_ImageCol[0]->set(     _color_map());
   Sh.h_ImageNrm[0]->set(    _normal_map());
#if DX9
   Sh.h_ImageNrm[1]->set(    _bump      ());
#endif
   Sh.h_ImageRfl[0]->set(_reflection_map());

   if(WaterMtrlLast!=this)
   {
      WaterMtrlLast=this;

   #if DX9
      if(_bump)SPSet("WaterBumpDot", (_bump->hwType()==IMAGE_L8 || _bump->hwType()==IMAGE_F32) ? Vec4(1, 0, 0, 0) : Vec4(0, 0, 0, 1));
   #endif
      // TODO: optimize (struct Water)
      SPSet("WaterScaleDif"    , scale_color  );
      SPSet("WaterScaleNrm"    , scale_normal );
      SPSet("WaterScaleBump"   , scale_bump   );
      SPSet("WaterRfr"         , refract      );
      SPSet("WaterRgh_2"       , rough*0.5f   );
      SPSet("WaterRflFake"     , reflect_tex  );
      SPSet("WaterSpc"         , specular     );
      SPSet("WaterWave"        , wave_scale   );
      SPSet("WaterFresnelPow"  , fresnel_pow  );
      SPSet("WaterFresnelRough", fresnel_rough);
      SPSet("WaterFresnelColor", fresnel_color);
      SPSet("WaterCol"         , color        );
      SPSet("WaterDns"         , Vec2(Mid(density, 0.0f, 1-EPS_GPU), density_add)); // avoid 1 in case "Pow(1-density, ..)" in shader would cause NaN or slow-downs
      if(Renderer._mirror_rt)
      {
         SPSet("WaterRfl"   , reflect_world     );
         SPSet("WaterRfrRfl", refract_reflection);
      }else
      {
         SPSet("WaterRfl"   , 0);
         SPSet("WaterRfrRfl", 0);
      }
   }
   MaterialClear();
}
/******************************************************************************/
Bool WaterMtrl::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<density<<density_add<<density_underwater<<density_underwater_add<<scale_color<<scale_normal<<scale_bump<<rough<<reflect_tex<<reflect_world<<refract<<refract_reflection<<refract_underwater<<specular<<wave_scale<<fresnel_pow<<fresnel_rough;
   f<<fresnel_color<<color<<color_underwater0<<color_underwater1;

   f._putStr(     _color_map.name(path));
   f._putStr(    _normal_map.name(path));
   f._putStr(_reflection_map.name(path));

   return f.ok();
}
Bool WaterMtrl::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>density>>density_add>>density_underwater>>density_underwater_add>>scale_color>>scale_normal>>scale_bump>>rough>>reflect_tex>>reflect_world>>refract>>refract_reflection>>refract_underwater>>specular>>wave_scale>>fresnel_pow>>fresnel_rough;
         f>>fresnel_color>>color>>color_underwater0>>color_underwater1;
              colorMap(ImagePtr().require(f._getStr(), path));
             normalMap(ImagePtr().require(f._getStr(), path));
         reflectionMap(ImagePtr().require(f._getStr(), path));
         if(f.ok())return true;
      }break;
   }
   reset(); return false;
}
Bool WaterMtrl::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f, _GetPath(name)) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool WaterMtrl::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f, _GetPath(name));
   reset(); return false;
}
/******************************************************************************/
// WATER PLANE
/******************************************************************************/
WaterClass::WaterClass()
{
   draw                 =false;
   reflection_allow     =!MOBILE;
   reflection_shadows   =false;
   reflection_resolution=2;
   plane.normal.y       =1;
  _max_1_light          =true;
  _draw_plane_surface   =false;
  _use_secondary_rt     =false;
  _began                =false;
  _swapped_ds           =false;
  _reflect_renderer     =RT_SIMPLE;
}
void WaterClass::del()
{
  _mshr     .del();
  _bump_temp.del();
}
void WaterClass::create()
{
   del();

   // create mesh
   MeshBase mshb; mshb.createPlane(80, 130).scaleMove(Vec(1, -1, 1), Vec(0, 1, 0));

   REPA(mshb.vtx)
   {
      Vec &pos=mshb.vtx.pos(i);
      if(  pos.x<=EPS || pos.x>=1-EPS
      ||   pos.y<=EPS || pos.y>=1-EPS)pos.xy=(pos.xy-0.5f)*6+0.5f;
   }
  _mshr.create(mshb);
}
/******************************************************************************/
WaterClass& WaterClass::reflectionRenderer(RENDER_TYPE type)
{
   Clamp(type, RENDER_TYPE(0), RENDER_TYPE(RT_NUM-1));
   if(type==RT_DEFERRED && D.deferredUnavailable())return T;
   if(type!=_reflect_renderer)
   {
     _reflect_renderer=type;
      if(type==RT_DEFERRED && D.deferredMSUnavailable())D.samples(1); // disable multi-sampling if we can't support it
      D.setShader(); // needed because shaders are set only for current renderer type
   }
   return T;
}
WaterClass& WaterClass::max1Light(Bool on) {/*if(_max_1_light!=on)*/_max_1_light=on; return T;}
/******************************************************************************/
void WaterClass::prepare() // this is called at the start
{
   WaterMtrlLast=null;

  _under_mtrl        =null;
  _draw_plane_surface=false;

   // check visibility for water plane
   if(draw)
   {
      Matrix3 matrix; matrix.setUp(plane.normal);
      Flt     size=D.viewRange();
      VecD    p   =PointOnPlane(CamMatrix.pos, plane.pos, plane.normal);

      under(plane, T);
      if(Frustum(Box(size*2, wave_scale, size*2), MatrixM(matrix, p)))
      {
        _draw_plane_surface=true;
        _quad.set(p+size*(matrix.z-matrix.x), p+size*(matrix.x+matrix.z), p+size*(matrix.x-matrix.z), p+size*(-matrix.x-matrix.z));
      }
   }

   // !! it is important to call this as the first thing during rendering, because 'Game.WorldManager.draw' assumes so, if this needs to be changed then rework 'Game.WorldManager.draw' !!
   // test for under-water on all other water elements
   {
     _mode=MODE_UNDERWATER; Renderer.mode(RM_WATER); Renderer._render();
     _mode=MODE_DRAW      ;
   }

   //  set draw plane surface
   if(_draw_plane_surface)
   {
      if(_under_mtrl && _under_step>=1) // totally under water
      {
        _draw_plane_surface=false;
      }else
      {
         VecD left =FullScreenToPos(Vec2(D.viewRect().min.x, 0), D.viewRange()),
              right=FullScreenToPos(Vec2(D.viewRect().max.x, 0), D.viewRange());

         left =PointOnPlaneRay(left , plane.pos, plane.normal, CamMatrix.y);
         right=PointOnPlaneRay(right, plane.pos, plane.normal, CamMatrix.y);

         Vec2 l2=PosToFullScreen(left ),
              r2=PosToFullScreen(right);

         Rect rect=D.viewRect();
         Flt  dot =Dot(CamMatrix.y, plane.normal);

         if(dot>EPS)
         {
               rect.max.y=Max(l2.y, r2.y);
            if(rect.max.y<rect.min.y)_draw_plane_surface=false;else MIN(rect.max.y, D.viewRect().max.y);
         }else
         if(dot<-EPS)
         {
               rect.min.y=Min(l2.y, r2.y);
            if(rect.min.y>rect.max.y)_draw_plane_surface=false;else MAX(rect.min.y, D.viewRect().min.y);
         }

       /*l2.draw(RED);
         r2.draw(RED);
         rect.draw(0x30FFFFFF);*/

         if(_draw_plane_surface)
         {
            // full    range is D.viewRect().min.y .. D.viewRect().max.y
            // drawing range is       rect  .min.y ..       rect  .max.y
            Flt y_min_frac=LerpR(D.viewRect().min.y, D.viewRect().max.y, rect.min.y),
                y_max_frac=LerpR(D.viewRect().min.y, D.viewRect().max.y, rect.max.y);
         #if 1
           _y_mul_add.x=y_max_frac-y_min_frac;
           _y_mul_add.y=1-y_max_frac;
         #else
           _y_mul_add.x=1;
           _y_mul_add.y=0;
         #endif

            if(reflection_allow && reflect_world>EPS_COL)Renderer.requestMirror(plane, 1, reflection_shadows, reflection_resolution);
            WS.load();
         }
      }
   }

   // test for lake reflections
   if(reflection_allow && reflect_world>EPS_COL && !Renderer._mirror_want)
      if(!(_under_mtrl && _under_step>=1))
   {
     _mode=MODE_REFLECTION; Renderer.mode(RM_WATER); Renderer._render();
     _mode=MODE_DRAW      ;
   }
}
/******************************************************************************/
void WaterClass::setEyeViewport()
{
   Renderer.setEyeViewport();
   if(Renderer._mirror_rt)
   {
      Vec2 scale=1,
           offs =0;
      if(Renderer._stereo)
      {
         scale.x=2;
         if(Renderer._eye)offs.x-=1;
         offs.x-=ProjMatrixEyeOffset[Renderer._eye]*0.5f;
      }
      SPSet("WaterRflMulAdd", Vec4(scale, offs));
   }else
   {
      SPSet("WaterRflMulAdd", Vec4(1, 1, 0, 0));
   }
}
/******************************************************************************/
void WaterClass::begin()
{
   if(!_began)
   {
     _began=true;

      WS.load();
      Renderer._has_glow=true;
      D.alpha(ALPHA_NONE);

      if(_use_secondary_rt)
      {
        _swapped_ds=false;
         Renderer._water_col.get  (ImageRTDesc(Renderer._col->w(), Renderer._col->h(),                                    IMAGERT_RGBA)); // here Alpha is used for Glow
         Renderer._water_nrm.get  (ImageRTDesc(Renderer._col->w(), Renderer._col->h(), D.signedNrmRT() ? IMAGERT_RGBA_S : IMAGERT_RGBA)); // here Alpha is used for Specular
         Renderer._water_ds .getDS(            Renderer._col->w(), Renderer._col->h(), 1, false);

         if(Renderer.stage)switch(Renderer.stage)
         {
            case RS_WATER_COLOR : Renderer._water_col->clearFull(); break;
            case RS_WATER_NORMAL: Renderer._water_nrm->clearFull(); break;
         }

         // set RT's and depth buffer
         if(Shader *shader=Sh.h_SetDepth) // if we can copy depth buffer from existing solid's, then do it, to prevent drawing water pixels if they're occluded
         {
         #if DX9
            Renderer.set(Renderer._water_col(), Renderer._water_nrm(), null, null, Renderer._water_ds(), true); // DX9 always requires a RT set
            D.colWrite(0, 0);
            D.colWrite(0, 1);
         #else
            Renderer.set(null, Renderer._water_ds(), true);
         #endif
            D.depthLock  (true); D.depthFunc(FUNC_ALWAYS); D.stencil(STENCIL_ALWAYS_SET, 0); shader->draw(Renderer._ds_1s);
            D.depthUnlock(    ); D.depthFunc(FUNC_LESS  ); D.stencil(STENCIL_NONE         );
         #if DX9
            D.colWrite(COL_WRITE_RGBA, 0);
            D.colWrite(COL_WRITE_RGBA, 1);
         #else
          //Renderer.set(Renderer._water_col(), Renderer._water_nrm(), null, null, Renderer._water_ds(), true); don't set, instead swap first and set later
         #endif
           _swapped_ds=Renderer.swapDS1S(Renderer._water_ds); // try to swap DS to put existing stencil values into '_water_ds' because we will write water depths onto '_water_ds' and we want to use it later instead of '_ds_1s' so we want all stencil values to be kept
            Renderer.set(Renderer._water_col(), Renderer._water_nrm(), null, null, Renderer._water_ds(), true);
         }else // if we can't copy then just clear it
         {
            Renderer.set(Renderer._water_col(), Renderer._water_nrm(), null, null, Renderer._water_ds(), true);
            D.clearDS();
         }
      }else
      {
         if(Lights.elms() && Lights[0].type==LIGHT_DIR)Lights[0].dir.set();else LightDir(Vec(0,-1,0), VecZero, 0).set();
         // we're going to draw water on top of existing RT, including refraction, so we need to have a color copy of what's underwater (solid) for the refraction, also we want to do softing so we need to backup depth because we can't read and write to depth in the same time
         Renderer._water_col.get(ImageRTDesc(Renderer._col->w(), Renderer._col->h(), GetImageRTType(Renderer._col->type()))); // create RT for the copy
         Renderer._col->copyHw(*Renderer._water_col, false, D.viewRect()); // copy
         if(_shader_soft)
         {
            Renderer._water_ds.get(ImageRTDesc(Renderer._col->w(), Renderer._col->h(), IMAGERT_F32));
            Renderer._ds_1s->copyHw(*Renderer._water_ds, false, D.viewRect());
         }
         setImages(Renderer._water_col(), Renderer._water_ds());
         Renderer.set(Renderer._col(), Renderer._ds(), true);
      }

      SetOneMatrix();
      D.wire      (Renderer.wire);
      D.depth     (true);
      D.cull      (true);
      D.sampler3D (    );
      D.stencil   (STENCIL_WATER_SET, STENCIL_REF_WATER);
      SPSet("WaterFlow", Time.time()*3);
      SPSet("WaterOfs" , _offset      );
      Rect uv=D.screenToUV(D.viewRect()); // UV
      if(Renderer._mirror_rt)
      {
         uv=Round(uv*Renderer._mirror_rt->size()); // convert to RectI of RT size
         uv.extend(-0.5f)/=Renderer._mirror_rt->size(); // decrease half pixel to avoid tex filtering issues and convert back to UV
         SPSet("WaterPlnPos", Renderer._mirror_plane.pos   *CamMatrixInv      );
         SPSet("WaterPlnNrm", Renderer._mirror_plane.normal*CamMatrixInv.orn());
      }
      SPSet("WaterClamp", uv);
      setEyeViewport();
   }
}
void WaterClass::end()
{
   if(_began)
   {
     _began=false;

      D.wire     (false);
      D.sampler2D();
      D.stencil  (STENCIL_NONE);

      if(!_use_secondary_rt)
      {
         endImages();
         Renderer._water_col.clear();
         Renderer._water_ds .clear();
         if(Renderer._ds!=Renderer._ds_1s && Renderer._ds_1s) // if we've drawn to MSAA ds, then we need to setup 1S DS
         {
            Renderer.set(null, Renderer._ds_1s(), true);
            D.alpha(ALPHA_NONE);
            D.depthLock  (true); Sh.h_ResolveDepth->draw(Renderer._ds); // here we can keep FUNC_LESS because 1S DS was already set before and we just need to apply water on top
            D.depthUnlock(    );
         }
      }
   }
}
/******************************************************************************/
void WaterClass::under(C PlaneM &plane, WaterMtrl &mtrl)
{
   Flt step=(Dist(CamMatrix.pos, plane)-D.viewFromActual())/-WATER_SMOOTH+1;
   if( step>EPS_COL)
   {
      if(!_under_mtrl || step>_under_step){_under_mtrl=&mtrl; _under_step=step; _under_plane=plane; _under_plane.pos+=_under_plane.normal*(D.viewFromActual()+WATER_SMOOTH);}
   }
}
/******************************************************************************/
void WaterClass::setImages(Image *src, Image *depth)
{
   Sh.h_ImageCol[1]->set(Renderer._mirror_rt()); Sh.h_ImageCol[1]->_sampler=&SamplerLinearClamp; // reflection
   Sh.h_ImageCol[2]->set(          src        ); Sh.h_ImageCol[2]->_sampler=&SamplerLinearClamp; // solid underwater
   Sh.h_ImageDet[0]->set(          depth      );                                                 // solid depth
}
void WaterClass::endImages()
{
   Sh.h_ImageCol[1]->_sampler=null;
   Sh.h_ImageCol[2]->_sampler=null;
   MaterialClear();
}
/******************************************************************************/
Bool WaterClass::ocean()
{
   #define EPS_WAVE_SCALE 0.001f // 1 mm
   return _bump && wave_scale>EPS_WAVE_SCALE;
}
Shader* WaterClass::shader()
{
   Bool fake_reflect=(_reflection_map!=null);
   return _use_secondary_rt ? (ocean() ? WS.h_Ocean  : WS.h_Lake )[fake_reflect]
                            : (ocean() ? WS.h_OceanL : WS.h_LakeL)[fake_reflect][_shader_shadow][_shader_soft];
}
/******************************************************************************/
void WaterClass::drawSurfaces()
{
   // these are used only when '_use_secondary_rt' is disabled
  _shader_shadow=((Lights.elms() && Lights[0].type==LIGHT_DIR && Lights[0].shadow) ? D.shadowMapNumActual() : 0);
  _shader_soft  =Renderer.canReadDepth();

   REPS(Renderer._eye, Renderer._eye_num)
   {
      setEyeViewport();

      if(_draw_plane_surface)
         if(Shader *shader=T.shader())
      {
         begin();
         set  ();

         if(ocean())
         {
            SPSet("WaterPlnPos" , plane.pos   *CamMatrixInv      );
            SPSet("WaterPlnNrm" , plane.normal*CamMatrixInv.orn());
            SPSet("WaterYMulAdd", _y_mul_add                     );
            shader->begin(); _mshr.set().drawFull();
            ShaderEnd();
         }else
         {
            VI.shader(shader);
            VI.cull  (true  );
            VI.quad  (_quad );
            VI.end   (      );
         }
      }

      if(!(_under_mtrl && _under_step>=1)) // don't draw any surfaces when totally under water
      {
         Renderer.mode(RM_WATER); Renderer._render();
      }
   }

   end();
}
/******************************************************************************/
WaterClass& WaterClass::update(C Vec2 &vel)
{
  _offset+=vel*Time.d();
   return T;
}
/******************************************************************************/
// WATER MESH
/******************************************************************************/
void WaterMesh::zero()
{
   depth=0;
  _lake =false;
  _box.zero();
}
WaterMesh::WaterMesh() {zero();}
void WaterMesh::del()
{
  _mshb.del();
  _mshr.del();
  _material.clear();
   zero();
}
void WaterMesh::create(C MeshBase &src, Bool lake, Flt depth, C WaterMtrlPtr &material)
{
   T._lake    =lake;
   T. depth   =depth;
   T._material=material;
  _mshb.create(src, VTX_POS|VTX_TEX0|TRI_IND|QUAD_IND).getBox(_box);
  _mshr.create(_mshb); // 'mshb' is kept for testing 'under'
   WS.load();
}
/******************************************************************************/
Bool WaterMesh::under(C Vec &pos, Flt *depth)C
{
   if(pos.x>=_box.min.x && pos.x<=_box.max.x
   && pos.z>=_box.min.z && pos.z<=_box.max.z
   && pos.y<=_box.max.y && pos.y>=_box.min.y-T.depth)
   {
      Bool flat=(_box.h()<=0.01f);

      // per face precision
      Vec2 pos2=pos.xz();
    C Vec *p   =_mshb.vtx.pos();
      REPA(_mshb.tri)
      {
       C VecI &ind=_mshb.tri.ind(i);
         Tri2  tri(p[ind.x].xz(), p[ind.y].xz(), p[ind.z].xz());
         if(Cuts(pos2, tri))
         {
            if(flat)
            {
               if(depth)*depth=_box.max.y-pos.y;
               return true;
            }else
            {
               Vec blend=TriBlend(pos2, tri);
               Flt water=p[ind.x].y*blend.x + p[ind.y].y*blend.y + p[ind.z].y*blend.z,
                   d    =water-pos.y;
               if( d>=0 && d<T.depth)
               {
                  if(depth)*depth=d;
                  return true;
               }
               return false;
            }
         }
      }
      REPA(_mshb.quad)
      {
       C VecI4 &ind=_mshb.quad.ind(i);
         Quad2  quad(p[ind.x].xz(), p[ind.y].xz(), p[ind.z].xz(), p[ind.w].xz());
         if(Cuts(pos2, quad))
         {
            if(flat)
            {
               if(depth)*depth=_box.max.y-pos.y;
               return true;
            }else
            {
               Vec blend=TriBlend(pos2, quad.tri013());
               Flt water=p[ind.x].y*blend.x + p[ind.y].y*blend.y + p[ind.w].y*blend.z,
                   d    =water-pos.y;
               if( d>=0 && d<T.depth)
               {
                  if(depth)*depth=d;
                  return true;
               }
               return false;
            }
         }
      }
   }
   return false;
}
/******************************************************************************/
Shader* WaterMesh::shader()C
{
   if(WaterMtrl *mtrl=getMaterial())
   {
      Bool fake_reflect=(mtrl->_reflection_map!=null);
      return Water._use_secondary_rt ? (_lake ? WS.h_Lake  : WS.h_River )[fake_reflect]
                                     : (_lake ? WS.h_LakeL : WS.h_RiverL)[fake_reflect][Water._shader_shadow][Water._shader_soft];
   }
   return null;
}
/******************************************************************************/
void WaterMesh::draw()C
{
   if(WaterMtrl *mtrl=getMaterial())switch(Water._mode)
   {
      case WaterClass::MODE_DRAW:
      {
         if(Frustum(_box) && _mshr.is())
            if(Shader *shader=T.shader())
         {
            Water  .begin();
            mtrl  ->set  ();
            shader->begin(); _mshr.set().drawFull();
            ShaderEnd();
         }
      }break;

      case WaterClass::MODE_REFLECTION:
      {
         if(_box.h()<=0.01f /*&& mtrl->reflect>EPS_COL*/) // for reflections accept only flat waters, 'reflect' is taken globally
            if(CamMatrix.pos.y>=_box.min.y) // if camera is above surface
               if(Frustum(_box) && _mshr.is())
                  Renderer.requestMirror(PlaneM(VecD(0, _box.max.y, 0), Vec(0, 1, 0)), 0, Water.reflection_shadows, Water.reflection_resolution);
      }break;

      case WaterClass::MODE_UNDERWATER:
      {
         if(_mshb.vtxs())
         {
            Flt  depth, eps=D.viewFromActual()+WATER_SMOOTH;
            VecD test=CamMatrix.pos; test.y-=eps;
            if(under(test, &depth))
            {
               test.y+=depth;
               Water.under(PlaneM(test, Vec(0, 1, 0)), *mtrl);
            }
         }
      }break;
   }
}
/******************************************************************************/
Bool WaterMesh::save(File &f, CChar *path)C
{
   f.cmpUIntV(0); // version
   f<<_lake<<depth<<_box;
   if(_mshb.save(f))
   if(_mshr.save(f))
   {
      f._putStr(_material.name(path));
      return f.ok();
   }
   return false;
}
Bool WaterMesh::load(File &f, CChar *path)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>_lake>>depth>>_box;
         if(_mshb.load(f))
         if(_mshr.load(f))
         {
           _material.require(f._getStr(), path);
            if(f.ok())
            {
               WS.load();
               return true;
            }
         }
      }break;
   }
   del(); return false;
}
/******************************************************************************/
// WATER DROPS
/******************************************************************************
struct WaterDrops
{
   Int    num   ; // number of water drops
   Vec   *pos   , // position
         *nrm   , // normal  (1st random direction)
         *tan   ; // tangent (2nd random direction)
   Flt   *time  , // time offset
         *blend ; // bone blend value
   VecB4 *matrix; // bone matrix index
   Image *image ; // drop texture

   // manage
   WaterDrops& del   (                                              ); // delete
   WaterDrops& create(Image &image,Int num                          ); // create with 'num' drops
   WaterDrops& create(Image &image,Int num,Shape   *shape,Int shapes); // create from shapes
   WaterDrops& create(Image &image,Int num,Ragdoll &ragdoll         ); // create from ragdoll

   // operations
   void setSkin(Skeleton &skeleton); // recalculate bone skinning values according to 'skeleton'

   // draw
   void draw(                           ); // render with  active     matrix
   void draw(Matrix           &matrix   ); // render with 'matrix'    matrix
   void draw(AnimatedSkeleton &anim_skel); // render with 'anim_skel' matrixes

  ~WaterDrops() {del ( );}
   WaterDrops() {Zero(T);}

   NO_COPY_CONSTRUCTOR(WaterDrops);
};
/******************************************************************************
WaterDrops& WaterDrops::del()
{
   Free(pos);
   Free(nrm);
   Free(tan);
   Free(time);
   Free(blend);
   Free(matrix);
   Zero(T); return T;
}
WaterDrops& WaterDrops::create(Image &image,Int num)
{
   del();

   T.image=&image;
   T.num  = num  ;
   Alloc(pos ,num);
   Alloc(nrm ,num);
   Alloc(tan ,num);
   Alloc(time,num);
   return T;
}
WaterDrops& WaterDrops::create(Image &image,Int num,Shape *shape,Int shapes)
{
   create(image,num);

   Box  box;
   Bool box_set=false;
   REP(shapes)
   {
      Box    b;
      Shape &s=shape[i];
      if(s.type==SHAPE_BALL)
      {
         Ball &ball=s.ball;
         b.set(ball.pos-ball.r,ball.pos+ball.r);
      }else
      if(s.type==SHAPE_CAPSULE)
      {
         Capsule &capsule=s.capsule;
         Vec      up     =capsule.up*(capsule.h*0.5f-capsule.r);
         b.from(capsule.pos-up,capsule.pos+up).extend(capsule.r);
      }else continue;
      if(!box_set){box_set=true; box=b;}else box|=b;
   }
   REP(num)
   {
      pos [i].zero();
      nrm [i]=Random  (Ball(1),false)*Random.f(1/1.2f,1.2f)*0.008f;
      tan [i]=Random  (Ball(1),false)*Random.f(1/1.2f,1.2f)*0.008f;
      time[i]=Random.f(10);
      Vec &p=pos[i];
      REPD(a,256)
      {
         p=Random(box);
         Bool cut=false;
         REP(shapes)
         {
            Shape &s=shape[i];
            if(s.type==SHAPE_BALL    && Cuts(p,s.ball   )){cut=true; break;}else
            if(s.type==SHAPE_CAPSULE && Cuts(p,s.capsule)){cut=true; break;}
         }
         if(cut)break;
      }
   }
   return T;
}
WaterDrops& WaterDrops::create(Image &image,Int num,Ragdoll &ragdoll)
{
   return create(image,num,ragdoll.shape,ragdoll.bones);
}
/******************************************************************************
void WaterDrops::setSkin(Skeleton &skeleton)
{
   if(!blend )Alloc(blend ,num);
   if(!matrix)Alloc(matrix,num);
   REP(num)skeleton.getSkin(pos[i],blend[i],matrix[i]);
}
/******************************************************************************
void WaterDrops::draw()
{
   VI.image    (image);
   VI.technique(blend ? Sh.h_RndrWaterdS : Sh.h_RndrWaterd);
   REP(num)
   {
      Flt t1=Sin(Time.time()*1.5f+time[i]),
          t2=Sin(Time.time()*1.7f+time[i]);
      Vec p=pos[i]+t1*nrm[i]+t2*tan[i];
      if(blend)VI.image(WHITE,0.020f,p,blend[i],matrix[i]);
      else     VI.image(WHITE,0.020f,p);
   }
   VI.end();
}
void WaterDrops::draw(Matrix &matrix)
{
   SetOneMatrix(matrix);
   draw();
}
void WaterDrops::draw(AnimatedSkeleton &anim_skel)
{
   anim_skel.setMatrix();
   draw();
}
/******************************************************************************/
}
/******************************************************************************/
