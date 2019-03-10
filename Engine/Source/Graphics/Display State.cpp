/******************************************************************************/
#include "stdafx.h"
#include "../Shaders/!Header CPU.h"
namespace EE{
/******************************************************************************

   TODO: on DX11 states are activated ('set') for each single state change, while they should be activated only after all single changes

/******************************************************************************/
#define DEPTH_VALUE(x) (REVERSE_DEPTH ? -(x) : (x))

#if DX9 || GL
   #define DEPTH_BIAS_SHADOW  DEPTH_VALUE( 0.0f           )
   #define DEPTH_BIAS_OVERLAY DEPTH_VALUE(-1.0f/(1ull<<24)) // 24-bit depth buffer
#elif DX11
   #define DEPTH_BIAS_SHADOW  DEPTH_VALUE( 0)
   #define DEPTH_BIAS_OVERLAY DEPTH_VALUE(-1)
#endif

#define SLOPE_SCALED_DEPTH_BIAS_SHADOW  DEPTH_VALUE(SQRT2)
#define SLOPE_SCALED_DEPTH_BIAS_OVERLAY DEPTH_VALUE(-1.0f)
/******************************************************************************/
#if DX9
static Bool AllowLineSmooth;
#elif DX11
struct BlendState
{
   ID3D11BlendState *state;
   
   void del()
   {
      if(state)
      {
       //SyncLocker locker(D._lock); if(state) lock not needed for DX11 'Release'
            {if(D.created())state->Release(); state=null;}
      }
   }
   void create(D3D11_BLEND_DESC &desc)
   {
    //SyncLocker locker(D._lock); lock not needed for DX11 'D3D'
      del();
      if(D3D)D3D->CreateBlendState(&desc, &state);
   }
   void set()
   {
      D3DC->OMSetBlendState(state, D._alpha_factor_v4.c, D._sample_mask);
   }
   BlendState() {state=null;}
  ~BlendState() {del();}
};
/******************************************************************************/
struct DepthState
{
   ID3D11DepthStencilState *state;

   void del()
   {
      if(state)
      {
       //SyncLocker locker(D._lock); if(state) lock not needed for DX11 'Release'
            {if(D.created())state->Release(); state=null;}
      }
   }
   void create(D3D11_DEPTH_STENCIL_DESC &desc)
   {
    //SyncLocker locker(D._lock); lock not needed for DX11 'D3D'
      del();
      if(D3D)D3D->CreateDepthStencilState(&desc, &state);
   }
   void set()
   {
      D3DC->OMSetDepthStencilState(state, D._stencil_ref);
   }
   DepthState() {state=null;}
  ~DepthState() {del();}
};
/******************************************************************************/
struct RasterizerState
{
   ID3D11RasterizerState *state;

   void del()
   {
      if(state)
      {
       //SyncLocker locker(D._lock); if(state) lock not needed for DX11 'Release'
            {if(D.created())state->Release(); state=null;}
      }
   }
   void create(D3D11_RASTERIZER_DESC &desc)
   {
    //SyncLocker locker(D._lock); lock not needed for DX11 'D3D'
      del();
      if(D3D)D3D->CreateRasterizerState(&desc, &state);
   }
   void set()
   {
      D3DC->RSSetState(state);
   }
   RasterizerState() {state=null;}
  ~RasterizerState() {del();}
};
/******************************************************************************/
// set order that first array sizes are those not pow2, and followed by pow2 (faster accessing)
static BlendState      BS   [  ALPHA_NUM]         ; // [AlphaMode]
static DepthState      DS   [STENCIL_NUM][2][2][8]; // [StencilMode][DepthUse][DepthWrite][DepthFunc]
static RasterizerState RS[3][   BIAS_NUM][2][2][2]; // [Cull][Bias][LineSmooth][Wire][Clip]
#elif GL
static Bool DepthAllow=true, DepthReal;
static Byte Col0WriteAllow=COL_WRITE_RGBA, Col0WriteReal=COL_WRITE_RGBA;
static UInt StencilFunc=GL_ALWAYS, StencilMask=~0;
#endif

#if !DX11
static STENCIL_MODE LastStencilMode;
#endif
/******************************************************************************/
// DEPTH / STENCIL
/******************************************************************************/
DisplayState::DisplayState()
{
  _cull            =false;
  _line_smooth     =false;
  _wire            =false;
  _clip            =false;
  _clip_allow      =true;
  _clip_real       =false;
  _clip_rect       .zero();
  _clip_recti      .zero();
  _clip_plane_allow=false;
  _clip_plane      .zero();
  _sampler2D       =true;
  _depth_lock      =false;
  _depth           =false;
  _depth_write     =true;
  _depth_func      =FUNC_LESS;
  _alpha           =ALPHA_BLEND;
  _stencil         =STENCIL_NONE;
  _stencil_ref     =0;
  _bias            =BIAS_ZERO;
  _col_write[0]    =COL_WRITE_RGBA;
  _col_write[1]    =COL_WRITE_RGBA;
  _col_write[2]    =COL_WRITE_RGBA;
  _col_write[3]    =COL_WRITE_RGBA;
  _sample_mask     =~0;
  _viewport        .zero();
  _alpha_factor    .zero();
  _alpha_factor_v4 .zero();
  _vf              =null;
}

void DisplayState::depthUnlock(       ) {           D._depth_lock=false;}
void DisplayState::depthLock  (Bool on) {depth(on); D._depth_lock=true ;}
/******************************************************************************/
#if DX9
void DisplayState::depth     (Bool on  ) {                          if(D._depth      !=on && !D._depth_lock)D3D->SetRenderState(D3DRS_ZENABLE     , D._depth      =on  );}
Bool DisplayState::depthWrite(Bool on  ) {Bool last=D._depth_write; if(D._depth_write!=on                  )D3D->SetRenderState(D3DRS_ZWRITEENABLE, D._depth_write=on  ); return last;}
void DisplayState::depthFunc (UInt func) {                          if(D._depth_func !=func                )D3D->SetRenderState(D3DRS_ZFUNC       , D._depth_func =func);}
void DisplayState::stencilRef(Byte ref ) {                          if(D._stencil_ref!=ref                 )D3D->SetRenderState(D3DRS_STENCILREF  , D._stencil_ref=ref );}
void DisplayState::stencil   (STENCIL_MODE mode, Byte ref) {stencil(mode); stencilRef(ref);}
#elif DX11
   #define D3D11_COMPARISON_FIRST D3D11_COMPARISON_NEVER
static void SetDS() {DS[D._stencil][D._depth][D._depth_write][D._depth_func-D3D11_COMPARISON_FIRST].set();}
void DisplayState::depth     (Bool on  ) {                          if(D._depth      !=on && !D._depth_lock   ){D._depth      =on  ;                  SetDS();}}
Bool DisplayState::depthWrite(Bool on  ) {Bool last=D._depth_write; if(D._depth_write!=on                     ){D._depth_write=on  ;                  SetDS();} return last;}
void DisplayState::depthFunc (UInt func) {                          if(D._depth_func !=func                   ){D._depth_func =func;                  SetDS();}}
void DisplayState::stencilRef(Byte ref ) {                          if(D._stencil_ref!=ref                    ){D._stencil_ref=ref ;                  SetDS();}}
void DisplayState::stencil   (STENCIL_MODE mode, Byte ref) {        if(D._stencil_ref!=ref || D._stencil!=mode){D._stencil_ref=ref ; D._stencil=mode; SetDS();}}
#elif GL
Bool DisplayState::depthWrite(Bool on  ) {Bool last=D._depth_write; if(D._depth_write!=on  )glDepthMask  (             D._depth_write=on              ); return last;}
void DisplayState::depthFunc (UInt func) {                          if(D._depth_func !=func)glDepthFunc  (             D._depth_func =func            );}
void DisplayState::stencilRef(Byte ref ) {                          if(D._stencil_ref!=ref )glStencilFunc(StencilFunc, D._stencil_ref=ref, StencilMask);}
void DisplayState::stencil   (STENCIL_MODE mode, Byte ref)
{
#if 1
   STENCIL_MODE old_mode=LastStencilMode; // remember old values
   Byte         old_ref =D._stencil_ref; D._stencil_ref=ref; // already change 'D._stencil_ref' because calling 'stencil' might use it
   stencil(mode);
   if(old_mode==LastStencilMode && old_ref!=ref)glStencilFunc(StencilFunc, ref, StencilMask); // if 'stencil' didn't change 'LastStencilMode' and we wanted to change 'D._stencil_ref' then manually change the 'ref'
#else
   stencil(mode); stencilRef(ref);
#endif
}
void DisplayState::depth(Bool on)
{
   if(D._depth!=on && !D._depth_lock)
   {
      D._depth=on;
   #if !IOS // on desktop OpenGL and OpenGL ES (except iOS) '_main' is always linked with '_main_ds', when setting "_main null" RT DS, '_main_ds' is set either way but with depth disabled
      on&=DepthAllow; if(on==DepthReal)return; DepthReal=on;
   #endif
      if(on)glEnable(GL_DEPTH_TEST);else glDisable(GL_DEPTH_TEST);
   }
}
void DisplayState::depthAllow(Bool on)
{
   if(DepthAllow!=on)
   {
      DepthAllow=on;
      on&=D._depth;
      if(DepthReal!=on)if(DepthReal=on)glEnable(GL_DEPTH_TEST);else glDisable(GL_DEPTH_TEST);
   }
}
#endif
/******************************************************************************/
void DisplayState::depth2DOn(Bool background)
{
   UInt func=(background ? FUNC_LESS_EQUAL : FUNC_GREATER);
#if DX11
   if(D._depth!=true || D._depth_write!=false || D._depth_func!=func)
   {
      D._depth      =true ;
      D._depth_write=false;
      D._depth_func =func ;
      SetDS();
   }
   D._depth_lock=true;
#else
   depthLock (true );
   depthWrite(false);
   depthFunc (func );
#endif
}
void DisplayState::depth2DOff()
{
   UInt func=FUNC_LESS_EQUAL;
#if DX11
   if(D._depth_write!=true || D._depth_func!=func)
   {
      D._depth_write=true;
      D._depth_func =func;
      SetDS();
   }
   D._depth_lock=false;
#else
   depthWrite (true);
   depthUnlock(    );
   depthFunc  (func);
#endif
}
/******************************************************************************/
void DisplayState::stencil(STENCIL_MODE stencil)
{
#if DX9
   if(D._stencil!=stencil)
   {
      D._stencil=stencil;
      if(!stencil)
      {
         D3D->SetRenderState(D3DRS_STENCILENABLE, false);
      }else
      {
         D3D->SetRenderState(D3DRS_STENCILENABLE, true);

         if(LastStencilMode!=stencil)switch(LastStencilMode=stencil)
         {
            case STENCIL_ALWAYS_SET:
               D3D->SetRenderState(D3DRS_STENCILMASK     , ~0                  );
               D3D->SetRenderState(D3DRS_STENCILWRITEMASK, ~0                  );
               D3D->SetRenderState(D3DRS_STENCILFUNC     , D3DCMP_ALWAYS       );
               D3D->SetRenderState(D3DRS_CCW_STENCILFUNC , D3DCMP_ALWAYS       );
               D3D->SetRenderState(D3DRS_STENCILZFAIL    , D3DSTENCILOP_KEEP   );
               D3D->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP   );
               D3D->SetRenderState(D3DRS_STENCILPASS     , D3DSTENCILOP_REPLACE);
               D3D->SetRenderState(D3DRS_CCW_STENCILPASS , D3DSTENCILOP_REPLACE);
            break;

            case STENCIL_TERRAIN_TEST:
               D3D->SetRenderState(D3DRS_STENCILMASK     , STENCIL_REF_TERRAIN);
               D3D->SetRenderState(D3DRS_STENCILWRITEMASK, 0                  );
               D3D->SetRenderState(D3DRS_STENCILFUNC     , D3DCMP_EQUAL       );
               D3D->SetRenderState(D3DRS_CCW_STENCILFUNC , D3DCMP_EQUAL       );
               D3D->SetRenderState(D3DRS_STENCILZFAIL    , D3DSTENCILOP_KEEP  );
               D3D->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP  );
               D3D->SetRenderState(D3DRS_STENCILPASS     , D3DSTENCILOP_KEEP  );
               D3D->SetRenderState(D3DRS_CCW_STENCILPASS , D3DSTENCILOP_KEEP  );
            break;

            case STENCIL_MSAA_SET:
               D3D->SetRenderState(D3DRS_STENCILMASK     , STENCIL_REF_MSAA    );
               D3D->SetRenderState(D3DRS_STENCILWRITEMASK, STENCIL_REF_MSAA    );
               D3D->SetRenderState(D3DRS_STENCILFUNC     , D3DCMP_ALWAYS       );
               D3D->SetRenderState(D3DRS_CCW_STENCILFUNC , D3DCMP_ALWAYS       );
               D3D->SetRenderState(D3DRS_STENCILZFAIL    , D3DSTENCILOP_KEEP   );
               D3D->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP   );
               D3D->SetRenderState(D3DRS_STENCILPASS     , D3DSTENCILOP_REPLACE);
               D3D->SetRenderState(D3DRS_CCW_STENCILPASS , D3DSTENCILOP_REPLACE);
            break;

            case STENCIL_MSAA_TEST:
               D3D->SetRenderState(D3DRS_STENCILMASK     , STENCIL_REF_MSAA );
               D3D->SetRenderState(D3DRS_STENCILWRITEMASK, 0                );
               D3D->SetRenderState(D3DRS_STENCILFUNC     , D3DCMP_EQUAL     );
               D3D->SetRenderState(D3DRS_CCW_STENCILFUNC , D3DCMP_EQUAL     );
               D3D->SetRenderState(D3DRS_STENCILZFAIL    , D3DSTENCILOP_KEEP);
               D3D->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
               D3D->SetRenderState(D3DRS_STENCILPASS     , D3DSTENCILOP_KEEP);
               D3D->SetRenderState(D3DRS_CCW_STENCILPASS , D3DSTENCILOP_KEEP);
            break;

            case STENCIL_EDGE_SOFT_SET:
               D3D->SetRenderState(D3DRS_STENCILMASK     , STENCIL_REF_EDGE_SOFT);
               D3D->SetRenderState(D3DRS_STENCILWRITEMASK, STENCIL_REF_EDGE_SOFT);
               D3D->SetRenderState(D3DRS_STENCILFUNC     , D3DCMP_ALWAYS        );
               D3D->SetRenderState(D3DRS_CCW_STENCILFUNC , D3DCMP_ALWAYS        );
               D3D->SetRenderState(D3DRS_STENCILZFAIL    , D3DSTENCILOP_KEEP    );
               D3D->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP    );
               D3D->SetRenderState(D3DRS_STENCILPASS     , D3DSTENCILOP_REPLACE );
               D3D->SetRenderState(D3DRS_CCW_STENCILPASS , D3DSTENCILOP_REPLACE );
            break;

            case STENCIL_EDGE_SOFT_TEST:
               D3D->SetRenderState(D3DRS_STENCILMASK     , STENCIL_REF_EDGE_SOFT);
               D3D->SetRenderState(D3DRS_STENCILWRITEMASK, 0                    );
               D3D->SetRenderState(D3DRS_STENCILFUNC     , D3DCMP_EQUAL         );
               D3D->SetRenderState(D3DRS_CCW_STENCILFUNC , D3DCMP_EQUAL         );
               D3D->SetRenderState(D3DRS_STENCILZFAIL    , D3DSTENCILOP_KEEP    );
               D3D->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP    );
               D3D->SetRenderState(D3DRS_STENCILPASS     , D3DSTENCILOP_KEEP    );
               D3D->SetRenderState(D3DRS_CCW_STENCILPASS , D3DSTENCILOP_KEEP    );
            break;

            case STENCIL_WATER_SET:
               D3D->SetRenderState(D3DRS_STENCILMASK     , STENCIL_REF_WATER   );
               D3D->SetRenderState(D3DRS_STENCILWRITEMASK, STENCIL_REF_WATER   );
               D3D->SetRenderState(D3DRS_STENCILFUNC     , D3DCMP_ALWAYS       );
               D3D->SetRenderState(D3DRS_CCW_STENCILFUNC , D3DCMP_ALWAYS       );
               D3D->SetRenderState(D3DRS_STENCILZFAIL    , D3DSTENCILOP_KEEP   );
               D3D->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP   );
               D3D->SetRenderState(D3DRS_STENCILPASS     , D3DSTENCILOP_REPLACE);
               D3D->SetRenderState(D3DRS_CCW_STENCILPASS , D3DSTENCILOP_REPLACE);
            break;

            case STENCIL_WATER_TEST:
               D3D->SetRenderState(D3DRS_STENCILMASK     , STENCIL_REF_WATER);
               D3D->SetRenderState(D3DRS_STENCILWRITEMASK, 0                );
               D3D->SetRenderState(D3DRS_STENCILFUNC     , D3DCMP_EQUAL     );
               D3D->SetRenderState(D3DRS_CCW_STENCILFUNC , D3DCMP_EQUAL     );
               D3D->SetRenderState(D3DRS_STENCILZFAIL    , D3DSTENCILOP_KEEP);
               D3D->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
               D3D->SetRenderState(D3DRS_STENCILPASS     , D3DSTENCILOP_KEEP);
               D3D->SetRenderState(D3DRS_CCW_STENCILPASS , D3DSTENCILOP_KEEP);
            break;

            case STENCIL_OUTLINE_SET:
               D3D->SetRenderState(D3DRS_STENCILMASK     , STENCIL_REF_OUTLINE );
               D3D->SetRenderState(D3DRS_STENCILWRITEMASK, STENCIL_REF_OUTLINE );
               D3D->SetRenderState(D3DRS_STENCILFUNC     , D3DCMP_ALWAYS       );
               D3D->SetRenderState(D3DRS_CCW_STENCILFUNC , D3DCMP_ALWAYS       );
               D3D->SetRenderState(D3DRS_STENCILZFAIL    , D3DSTENCILOP_KEEP   );
               D3D->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP   );
               D3D->SetRenderState(D3DRS_STENCILPASS     , D3DSTENCILOP_REPLACE);
               D3D->SetRenderState(D3DRS_CCW_STENCILPASS , D3DSTENCILOP_REPLACE);
            break;

            case STENCIL_OUTLINE_TEST:
               D3D->SetRenderState(D3DRS_STENCILMASK     , STENCIL_REF_OUTLINE);
               D3D->SetRenderState(D3DRS_STENCILWRITEMASK, 0                  );
               D3D->SetRenderState(D3DRS_STENCILFUNC     , D3DCMP_EQUAL       );
               D3D->SetRenderState(D3DRS_CCW_STENCILFUNC , D3DCMP_EQUAL       );
               D3D->SetRenderState(D3DRS_STENCILZFAIL    , D3DSTENCILOP_KEEP  );
               D3D->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP  );
               D3D->SetRenderState(D3DRS_STENCILPASS     , D3DSTENCILOP_KEEP  );
               D3D->SetRenderState(D3DRS_CCW_STENCILPASS , D3DSTENCILOP_KEEP  );
            break;
         }
      }
   }
#elif DX11
   if(D._stencil!=stencil){D._stencil=stencil; SetDS();}
#elif GL
   if(D._stencil!=stencil)
   {
      D._stencil=stencil;
      if(!stencil)
      {
         glDisable(GL_STENCIL_TEST);
      }else
      {
         glEnable(GL_STENCIL_TEST);

         if(LastStencilMode!=stencil)switch(LastStencilMode=stencil)
         {
            case STENCIL_ALWAYS_SET:
               glStencilFunc(StencilFunc=GL_ALWAYS, D._stencil_ref, StencilMask=~0); // set full mask
               glStencilOp  (GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

            case STENCIL_TERRAIN_TEST:
               glStencilFunc(StencilFunc=GL_EQUAL, D._stencil_ref, StencilMask=STENCIL_REF_TERRAIN);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_KEEP);
            break;

            case STENCIL_MSAA_SET:
               glStencilFunc(StencilFunc=GL_ALWAYS, D._stencil_ref, StencilMask=STENCIL_REF_MSAA);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

            case STENCIL_MSAA_TEST:
               glStencilFunc(StencilFunc=GL_EQUAL, D._stencil_ref, StencilMask=STENCIL_REF_MSAA);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_KEEP);
            break;

            case STENCIL_EDGE_SOFT_SET:
               glStencilFunc(StencilFunc=GL_ALWAYS, D._stencil_ref, StencilMask=STENCIL_REF_EDGE_SOFT);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

            case STENCIL_EDGE_SOFT_TEST:
               glStencilFunc(StencilFunc=GL_EQUAL, D._stencil_ref, StencilMask=STENCIL_REF_EDGE_SOFT);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_KEEP);
            break;

            case STENCIL_WATER_SET:
               glStencilFunc(StencilFunc=GL_ALWAYS, D._stencil_ref, StencilMask=STENCIL_REF_WATER);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

            case STENCIL_WATER_TEST:
               glStencilFunc(StencilFunc=GL_EQUAL, D._stencil_ref, StencilMask=STENCIL_REF_WATER);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_KEEP);
            break;

            case STENCIL_OUTLINE_SET:
               glStencilFunc(StencilFunc=GL_ALWAYS, D._stencil_ref, StencilMask=STENCIL_REF_OUTLINE);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_REPLACE);
            break;

            case STENCIL_OUTLINE_TEST:
               glStencilFunc(StencilFunc=GL_EQUAL, D._stencil_ref, StencilMask=STENCIL_REF_OUTLINE);
               glStencilOp  (GL_KEEP, GL_KEEP, GL_KEEP);
            break;
         }
      }
   }
#endif
}
/******************************************************************************/
// RASTERIZER
/******************************************************************************/
#if DX9
Bool DisplayState::lineSmooth(Bool      on  ) {Bool old=D._line_smooth; if(D._line_smooth!=on){D._line_smooth=on; if(D3D && AllowLineSmooth)D3D->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, on);} return old;}
void DisplayState::wire      (Bool      on  ) {if(D._wire!=on)D3D->SetRenderState(D3DRS_FILLMODE, (D._wire=on) ? D3DFILL_WIREFRAME : D3DFILL_SOLID);}
void DisplayState::cull      (Bool      on  ) {if(D._cull!=on)D3D->SetRenderState(D3DRS_CULLMODE, Renderer._cull_mode[D._cull=on]);}
void DisplayState::bias      (BIAS_MODE bias)
{
   if(D._bias!=bias)switch(D._bias=bias)
   {
      case BIAS_ZERO:
      {
         D3D->SetRenderState(D3DRS_DEPTHBIAS          , 0);
         D3D->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, 0);
      }break;

      case BIAS_SHADOW:
      {
         Flt f;
         f=             DEPTH_BIAS_SHADOW; D3D->SetRenderState(D3DRS_DEPTHBIAS          , (UInt&)f);
         f=SLOPE_SCALED_DEPTH_BIAS_SHADOW; D3D->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, (UInt&)f);
      }break;

      case BIAS_OVERLAY:
      {
         Flt f;
         f=             DEPTH_BIAS_OVERLAY; D3D->SetRenderState(D3DRS_DEPTHBIAS          , (UInt&)f);
         f=SLOPE_SCALED_DEPTH_BIAS_OVERLAY; D3D->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, (UInt&)f);
      }break;
   }
}
#elif DX11
static void SetRS() {RS[Renderer._cull_mode[D._cull]][D._bias][D._line_smooth][D._wire][D._clip_real].set();}
Bool DisplayState::lineSmooth(Bool      on  ) {Bool old=D._line_smooth; if(D._line_smooth!=on){D._line_smooth=on; if(D3DC)SetRS();} return old;}
void DisplayState::wire      (Bool      on  ) {if(D._wire!=on  ){D._wire=on  ; SetRS();}}
void DisplayState::cull      (Bool      on  ) {if(D._cull!=on  ){D._cull=on  ; SetRS();}}
void DisplayState::bias      (BIAS_MODE bias) {if(D._bias!=bias){D._bias=bias; SetRS();}}
#elif GL
Bool DisplayState::lineSmooth(Bool on)
{
   Bool old=D._line_smooth;
   if(D._line_smooth!=on)
   {
      D._line_smooth=on;
   #ifdef GL_LINE_SMOOTH
      if(on)glEnable(GL_LINE_SMOOTH);else glDisable(GL_LINE_SMOOTH);
   #endif
   }
   return old;
}
void DisplayState::wire(Bool on)
{
#if !GL_ES
	if(D._wire!=on)glPolygonMode(GL_FRONT_AND_BACK, (D._wire=on) ? GL_LINE : GL_FILL);
#endif
}
void DisplayState::cull  (Bool on) {if(D._cull!=on)if(D._cull=on)glEnable(GL_CULL_FACE);else glDisable(GL_CULL_FACE);}
void DisplayState::cullGL(       )
{
   Bool front=(Renderer()==RM_SHADOW || !Renderer.mirror());
   if(!D.mainFBO())front^=1;
   glFrontFace(front ? GL_CW : GL_CCW);
}
void DisplayState::bias(BIAS_MODE bias)
{
   if(D._bias!=bias)switch(D._bias=bias)
   {
      case BIAS_ZERO:
      {
         glDisable      (GL_POLYGON_OFFSET_FILL);
         glPolygonOffset(0, 0);
      }break;

      case BIAS_SHADOW:
      {
         glEnable       (GL_POLYGON_OFFSET_FILL);
         glPolygonOffset(SLOPE_SCALED_DEPTH_BIAS_SHADOW, DEPTH_BIAS_SHADOW);
      }break;

      case BIAS_OVERLAY:
      {
         glEnable       (GL_POLYGON_OFFSET_FILL);
         glPolygonOffset(SLOPE_SCALED_DEPTH_BIAS_OVERLAY, DEPTH_BIAS_OVERLAY);
      }break;
   }
}
#endif
/******************************************************************************/
static void SetClip()
{
   if(D._clip_real!=(D._clip && D._clip_allow))
   {
      D._clip_real^=1;
   #if DX9
      D3D->SetRenderState(D3DRS_SCISSORTESTENABLE, D._clip_real);
   #elif DX11
      SetRS();
   #elif GL
      if(D._clip_real)glEnable(GL_SCISSOR_TEST);else glDisable(GL_SCISSOR_TEST);
   #endif
   }
}
static void SetClipRect(C RectI &rect)
{
#if GL
   RectI r(rect.min.x, D.mainFBO() ? Renderer.resH()-rect.max.y : rect.min.y, rect.w(), rect.h()); // RectI(pos, size)
   #define rect r
#endif
   if(D._clip_recti!=rect)
   {
      D._clip_recti=rect; // !! Warning: for GL this is actually RectI(pos, size)
   #if DX9
      RECT rectangle; rectangle.left=rect.min.x; rectangle.right=rect.max.x; rectangle.top=rect.min.y; rectangle.bottom=rect.max.y;
      D3D->SetScissorRect(&rectangle);
   #elif DX11
      D3D11_RECT rectangle; rectangle.left=rect.min.x; rectangle.right=rect.max.x; rectangle.top=rect.min.y; rectangle.bottom=rect.max.y;
      D3DC->RSSetScissorRects(1, &rectangle);
   #elif GL
      glScissor(r.min.x, r.min.y, r.max.x, r.max.y); // glScissor(pos, size)
      #undef rect
   #endif
   }
}
static void SetClipRect()
{
   RectI recti=Renderer.screenToPixelI(D._clip_rect);
   if(recti.max.x<=recti.min.x || recti.max.y<=recti.min.y)recti.zero(); // if the rectangle is invalid then zero it, to prevent negative sizes in case drivers can't handle them properly
   SetClipRect(recti);
}
void DisplayState::clipForce(C RectI &rect) {D._clip_recti=rect;} // called only for DX9
void DisplayState::clip     (C Rect  *rect)
{
   if(rect)
   {
      if(!D._clip || D._clip_rect!=*rect)
      {
         D._clip     = true;
         D._clip_rect=*rect;
         SetClip    ();
         SetClipRect();
      }
   }else
   if(D._clip)
   {
      D._clip=false; SetClip();
   }
}
void DisplayState::clipAllow(C RectI &rect) // this gets called during shadow rendering
{
   SetClipRect(rect);
   D._clip_rect=Renderer.pixelToScreen(rect); // needed in case we call 'clip' later which checks if '_clip_rect' changes
   D._clip=D._clip_allow=true; SetClip(); // call after setting '_clip' and '_clip_allow'
}
void DisplayState::clipAllow(Bool on) // this gets called when a render target has changed
{
 //if(D._clip_allow!=on) we must always do below, because: we may have a different sized RT now, and need to recalculate clip rect. DX9 could have reset clip rect when changing RT's, for GL we may have changed between D.mainFBO for which we need different orientation of the clip rect
   {
      D._clip_allow=on; SetClip();
      if(D._clip_real)SetClipRect(); // call after 'SetClip' because we need '_clip_real'
   }
}
/******************************************************************************/
void DisplayState::clipPlane(Bool on)
{
#if DX9
   D3D->SetRenderState(D3DRS_CLIPPLANEENABLE, on);
#elif DX11
   D._clip_plane_allow=on;
   Sh.h_ClipPlane->set(on ? D._clip_plane : Vec4(0, 0, 0, 1));
#elif GL && defined GL_CLIP_DISTANCE0
   if(on)glEnable(GL_CLIP_DISTANCE0);else glDisable(GL_CLIP_DISTANCE0);
#endif
}
void DisplayState::clipPlane(C PlaneM &plane)
{
   VecD pos=plane.pos   *CamMatrixInv;
   Vec  nrm=plane.normal*CamMatrixInv.orn();
#if DX9
   Vec4    clip_plane(nrm, -Dot(pos, nrm));
   Matrix4 m=ProjMatrix; m.inverse().transpose(); clip_plane*=m;
   D3D->SetClipPlane(0, clip_plane.c);
#elif DX11
   D._clip_plane.set(nrm, -Dot(pos, nrm));
   clipPlane(D._clip_plane_allow);
#elif GL
   Vec4 clip_plane(nrm, -Dot(pos, nrm));
   Sh.h_ClipPlane->set(clip_plane);
#endif
}
/******************************************************************************/
// OUTPUT MERGER
/******************************************************************************/
ALPHA_MODE DisplayState::alpha(ALPHA_MODE alpha)
{
   ALPHA_MODE prev=D.alpha();
#if DX9
   if(D._alpha!=alpha)switch(D._alpha=alpha)
   {
      case ALPHA_NONE:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , false);
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
      break;

      case ALPHA_BLEND:
         // final_alpha = 1-(1-src_alpha)*(1-dest_alpha)
         // final_alpha = 1 - (1 - src_alpha - dest_alpha + src_alpha*dest_alpha)
         // final_alpha = 1 - 1 + src_alpha + dest_alpha - src_alpha*dest_alpha
         // final_alpha = src_alpha + dest_alpha - src_alpha*dest_alpha
         // final_alpha = src_alpha*(1-dest_alpha) + dest_alpha
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true);
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD      );
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_SRCALPHA   );
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
         D3D->SetRenderState(D3DRS_BLENDOPALPHA  , D3DBLENDOP_ADD       );
         D3D->SetRenderState(D3DRS_SRCBLENDALPHA , D3DBLEND_INVDESTALPHA);
         D3D->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE         );
      break;
      case ALPHA_BLEND_DEC:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true);
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD      );
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_SRCALPHA   );
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
         D3D->SetRenderState(D3DRS_BLENDOPALPHA  , D3DBLENDOP_ADD      );
         D3D->SetRenderState(D3DRS_SRCBLENDALPHA , D3DBLEND_ZERO       );
         D3D->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
      break;

      case ALPHA_ADD:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true );
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD);
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_ONE  );
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE  );
      break;
      case ALPHA_MUL:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true );
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD    );
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_DESTCOLOR);
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO     );
      break;

      case ALPHA_MERGE:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true);
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD);
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_ONE  );
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
         D3D->SetRenderState(D3DRS_BLENDOPALPHA  , D3DBLENDOP_ADD       );
         D3D->SetRenderState(D3DRS_SRCBLENDALPHA , D3DBLEND_INVDESTALPHA);
         D3D->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE         );
      break;
      case ALPHA_ADDBLEND_KEEP:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true);
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD   );
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_SRCALPHA);
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE     );
         D3D->SetRenderState(D3DRS_BLENDOPALPHA  , D3DBLENDOP_ADD);
         D3D->SetRenderState(D3DRS_SRCBLENDALPHA , D3DBLEND_ZERO );
         D3D->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE  );
      break;
      case ALPHA_ADD_KEEP:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true);
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD);
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_ONE  );
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE  );
         D3D->SetRenderState(D3DRS_BLENDOPALPHA  , D3DBLENDOP_ADD);
         D3D->SetRenderState(D3DRS_SRCBLENDALPHA , D3DBLEND_ZERO );
         D3D->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE  );
      break;

      case ALPHA_BLEND_FACTOR:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true);
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD      );
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_SRCALPHA   );
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
         D3D->SetRenderState(D3DRS_BLENDOPALPHA  , D3DBLENDOP_ADD      );
         D3D->SetRenderState(D3DRS_SRCBLENDALPHA , D3DBLEND_BLENDFACTOR);
         D3D->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
      break;
      case ALPHA_ADD_FACTOR:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true);
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD);
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_ONE  );
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE  );
         D3D->SetRenderState(D3DRS_BLENDOPALPHA  , D3DBLENDOP_ADD      );
         D3D->SetRenderState(D3DRS_SRCBLENDALPHA , D3DBLEND_BLENDFACTOR);
         D3D->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE        );
      break;
      case ALPHA_SETBLEND_SET:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true);
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD   );
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_SRCALPHA);
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO    );
         D3D->SetRenderState(D3DRS_BLENDOPALPHA  , D3DBLENDOP_ADD);
         D3D->SetRenderState(D3DRS_SRCBLENDALPHA , D3DBLEND_ONE  );
         D3D->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO );
      break;
      case ALPHA_FACTOR:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true );
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD         );
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_BLENDFACTOR   );
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVBLENDFACTOR);
      break;

      case ALPHA_INVERT:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true );
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD       );
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_INVDESTCOLOR);
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO        );
      break;

      case ALPHA_FONT:
         // final = font_color.rgb * alpha.rgb + (1-alpha.rgb)*dest
         // src_color (PS output) = alpha.rgb
         // blend_factor          = font_color.rgb
         //  src_blend            = blend_factor
         // dest_blend            = inv_src_color
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true );
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD      );
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_BLENDFACTOR);
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
      break;
      case ALPHA_FONT_DEC:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true);
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD      );
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_BLENDFACTOR);
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
         D3D->SetRenderState(D3DRS_BLENDOPALPHA  , D3DBLENDOP_ADD      );
         D3D->SetRenderState(D3DRS_SRCBLENDALPHA , D3DBLEND_ZERO       );
         D3D->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
      break;

      case ALPHA_NONE_ADD:
         D3D->SetRenderState(D3DRS_ALPHABLENDENABLE        , true);
         D3D->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
         D3D->SetRenderState(D3DRS_BLENDOP  , D3DBLENDOP_ADD);
         D3D->SetRenderState(D3DRS_SRCBLEND , D3DBLEND_ONE  );
         D3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO );
         D3D->SetRenderState(D3DRS_BLENDOPALPHA  , D3DBLENDOP_ADD);
         D3D->SetRenderState(D3DRS_SRCBLENDALPHA , D3DBLEND_ONE  );
         D3D->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE  );
      break;
   }
#elif DX11
   if(D._alpha!=alpha)BS[D._alpha=alpha].set();
#elif GL
   if(D._alpha!=alpha)switch(D._alpha=alpha)
   {
      case ALPHA_NONE:
         glDisable(GL_BLEND);
      break;

      case ALPHA_BLEND:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
      break;
      case ALPHA_BLEND_DEC:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
      break;

      case ALPHA_ADD:
         glEnable       (GL_BLEND);
         glBlendEquation(GL_FUNC_ADD);
         glBlendFunc    (GL_ONE, GL_ONE);
      break;
      case ALPHA_MUL:
         glEnable       (GL_BLEND);
         glBlendEquation(GL_FUNC_ADD);
         glBlendFunc    (GL_DST_COLOR, GL_ZERO);
      break;

      case ALPHA_MERGE:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
      break;
      case ALPHA_ADDBLEND_KEEP:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
      break;
      case ALPHA_ADD_KEEP:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
      break;

      case ALPHA_BLEND_FACTOR:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_CONSTANT_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
      case ALPHA_ADD_FACTOR:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_ONE, GL_ONE, GL_CONSTANT_ALPHA, GL_ONE);
      break;
      case ALPHA_SETBLEND_SET:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_SRC_ALPHA, GL_ZERO, GL_ONE, GL_ZERO);
      break;
      case ALPHA_FACTOR:
         glEnable       (GL_BLEND);
         glBlendEquation(GL_FUNC_ADD);
         glBlendFunc    (GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR);
      break;

      case ALPHA_INVERT:
         glEnable       (GL_BLEND);
         glBlendEquation(GL_FUNC_ADD);
         glBlendFunc    (GL_ONE_MINUS_DST_COLOR, GL_ZERO);
      break;

      case ALPHA_FONT:
         glEnable       (GL_BLEND);
         glBlendEquation(GL_FUNC_ADD);
         glBlendFunc    (GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR);
      break;
      case ALPHA_FONT_DEC:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
      break;

      case ALPHA_NONE_ADD:
         glEnable           (GL_BLEND);
         glBlendEquation    (GL_FUNC_ADD);
         glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
      break;
   }
#endif
   return prev;
}
void DisplayState::alphaFactor(C Color &factor)
{
   if(D._alpha_factor!=factor)
   {
      D._alpha_factor=factor;
	#if DX9
      D3D->SetRenderState(D3DRS_BLENDFACTOR, VecB4(factor.b, factor.g, factor.r, factor.a).u);
	#elif DX11
      D._alpha_factor_v4=factor.asVec4();
      BS[D._alpha].set();
   #elif GL
      glBlendColor(factor.r/255.0f, factor.g/255.0f, factor.b/255.0f, factor.a/255.0f);
   #endif
   }
}
/******************************************************************************/
void DisplayState::colWrite(Byte color_mask, Byte index)
{
   RANGE_ASSERT(index, D._col_write);
#if DX9
   if(D._col_write[index]!=color_mask)
   {
      D3DRENDERSTATETYPE out;
      switch(index)
      {
         case  0: out=D3DRS_COLORWRITEENABLE ; break;
         case  1: out=D3DRS_COLORWRITEENABLE1; break;
         case  2: out=D3DRS_COLORWRITEENABLE2; break;
         case  3: out=D3DRS_COLORWRITEENABLE3; break;
         default: return;
      }
      D3D->SetRenderState(out, D._col_write[index]=color_mask);
   }
#elif DX11
   // color writes are not supported on DX10+ implementation
#elif GL
   if(D._col_write[index]!=color_mask)
   {
      D._col_write[index]=color_mask;
   #if !IOS // on desktop OpenGL and OpenGL ES (except iOS) '_main' is always linked with '_main_ds', when setting "null _main_ds" RT DS, '_main' is set either way but with color writes disabled
      if(!index){color_mask&=Col0WriteAllow; if(color_mask==Col0WriteReal)return; Col0WriteReal=color_mask;}
   #endif
   #if GL_ES
      if(!index)glColorMask(FlagTest(color_mask, COL_WRITE_R), FlagTest(color_mask, COL_WRITE_G), FlagTest(color_mask, COL_WRITE_B), FlagTest(color_mask, COL_WRITE_A)); // TODO: 'glColorMaski' requires GLES 3.2 - https://www.khronos.org/registry/OpenGL-Refpages/es3/html/glColorMask.xhtml
   #else
      glColorMaski(index, FlagTest(color_mask, COL_WRITE_R), FlagTest(color_mask, COL_WRITE_G), FlagTest(color_mask, COL_WRITE_B), FlagTest(color_mask, COL_WRITE_A));
   #endif
   }
#endif
}
#if GL
void DisplayState::colWriteAllow(Byte color_mask) // this operates only on "index==0"
{
   if(Col0WriteAllow!=color_mask)
   {
      Col0WriteAllow=color_mask;
      color_mask&=D._col_write[0];
      if(Col0WriteReal!=color_mask)
      {
         Col0WriteReal=color_mask;
         glColorMask(FlagTest(color_mask, COL_WRITE_R), FlagTest(color_mask, COL_WRITE_G), FlagTest(color_mask, COL_WRITE_B), FlagTest(color_mask, COL_WRITE_A));
      }
   }
}
#endif
/******************************************************************************/
#if DX11
void DisplayState::sampleMask(UInt mask)
{
   if(mask!=D._sample_mask){D._sample_mask=mask; BS[D._alpha].set();}
}
#endif
/******************************************************************************/
void DisplayState::viewportForce(C RectI &rect) // this is only for DX9
{
   if(D._viewport!=rect)
   {
      D._viewport=rect;
   #if DX9
      if(Sh.h_PixelOffset)
      {
         Sh.h_PixelOffset->set(PixelOffset.set(-1.0f/rect.w(), 1.0f/rect.h()));
         SetProjMatrix(); // because 'ProjMatrix' in the shader is based on 'PixelOffset' in DX9, after changing it we need to update the 'ProjMatrix' shader value
      }
   #endif
   }
}
void DisplayState::viewport(C RectI &rect, Bool allow_proj_matrix_update)
{
   if(D._viewport!=rect)
   {
      D._viewport=rect;
   #if DX9
      D3DVIEWPORT9 vp;
      vp.X=D._viewport.min.x; vp.Width =D._viewport.w(); vp.MinZ=0;
      vp.Y=D._viewport.min.y; vp.Height=D._viewport.h(); vp.MaxZ=1;
      D3D->SetViewport(&vp);
      if(Sh.h_PixelOffset)
      {
         Sh.h_PixelOffset->set(PixelOffset.set(-1.0f/vp.Width, 1.0f/vp.Height));
         if(allow_proj_matrix_update)SetProjMatrix(); // because 'ProjMatrix' in the shader is based on 'PixelOffset' in DX9, after changing it we need to update the 'ProjMatrix' shader value, however set this only if we allow to do so, for example we can disallow this if we're going to update it manually after changing some other settings first
      }
   #elif DX11
      D3D11_VIEWPORT vp;
      vp.TopLeftX=D._viewport.min.x; vp.Width =D._viewport.w(); vp.MinDepth=0;
      vp.TopLeftY=D._viewport.min.y; vp.Height=D._viewport.h(); vp.MaxDepth=1;
      D3DC->RSSetViewports(1, &vp);
   #elif GL
      glViewport(D._viewport.min.x, D._viewport.min.y, D._viewport.w(), D._viewport.h());
   #endif
   }
}
/******************************************************************************/
void DisplayState::vf(GPU_API(IDirect3DVertexDeclaration9, ID3D11InputLayout, VtxFormatGL) *vf)
{
#if DX9
   if(D._vf!=vf)D3D->SetVertexDeclaration(D._vf=vf);
#elif DX11
   if(D._vf!=vf)D3DC->IASetInputLayout(D._vf=vf);
#elif GL
 // !! when using VAO's, this can be called only after setting the default 'VAO', because we use 'disable' expecting previous 'vf' to be in the same VAO !!
 //if(D._vf!=vf) OpenGL requires resetting this for each new VBO (for example Layered Clouds didn't work after displaying Sky, even though they have the same VF but different VBO)
   {
      if(D._vf   )D._vf->disable   ();
      if(D._vf=vf)D._vf-> enableSet();
   }
#endif
}
/******************************************************************************/
void DisplayState::sampler2D()
{
   D._sampler2D=true;
#if DX9
   D._sampler_filter[0]=D3DTEXF_LINEAR;
   D._sampler_filter[1]=D3DTEXF_LINEAR;
   D._sampler_filter[2]=D3DTEXF_LINEAR;
   D._sampler_address  =D3DTADDRESS_CLAMP;
   REP(16)
   {
      D3D->SetSamplerState(i, D3DSAMP_MINFILTER, D._sampler_filter[0]);
      D3D->SetSamplerState(i, D3DSAMP_MAGFILTER, D._sampler_filter[1]);
      D3D->SetSamplerState(i, D3DSAMP_MIPFILTER, D._sampler_filter[2]);
      D3D->SetSamplerState(i, D3DSAMP_ADDRESSU , D._sampler_address  );
      D3D->SetSamplerState(i, D3DSAMP_ADDRESSV , D._sampler_address  );
      D3D->SetSamplerState(i, D3DSAMP_ADDRESSW , D._sampler_address  );
   }
#elif DX11
   SamplerLinearClamp.setPS(SSI_DEFAULT);
#elif GL
 //D._sampler_filter[0]=GL_LINEAR; unused
 //D._sampler_filter[1]=GL_LINEAR; unused
 //D._sampler_filter[2]=GL_LINEAR; unused
   D._sampler_address  =GL_CLAMP_TO_EDGE;
#endif
}
void DisplayState::sampler3D()
{
   D._sampler2D=false;
#if DX9
   switch(D.texFilter())
   {
      case  0: D._sampler_filter[0]=D3DTEXF_LINEAR     ; break;
      case  1: D._sampler_filter[0]=D3DTEXF_LINEAR     ; break;
      default: D._sampler_filter[0]=D3DTEXF_ANISOTROPIC; break;
   }

   switch(D.texFilter())
   {
      case  0: D._sampler_filter[1]=D3DTEXF_POINT ; break;
      default: D._sampler_filter[1]=D3DTEXF_LINEAR; break;
   }

   UInt           aniso=Max(D.texFilter(), 1);
   D._sampler_filter[2]=(D.texMipFilter() ? D3DTEXF_LINEAR : D3DTEXF_POINT);
   D._sampler_address  =D3DTADDRESS_WRAP;

   REP(16)
   {
      D3D->SetSamplerState(i, D3DSAMP_MINFILTER    , D._sampler_filter[0]);
      D3D->SetSamplerState(i, D3DSAMP_MAGFILTER    , D._sampler_filter[1]);
      D3D->SetSamplerState(i, D3DSAMP_MIPFILTER    , D._sampler_filter[2]);
      D3D->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, aniso);
      D3D->SetSamplerState(i, D3DSAMP_ADDRESSU     , D._sampler_address);
      D3D->SetSamplerState(i, D3DSAMP_ADDRESSV     , D._sampler_address);
      D3D->SetSamplerState(i, D3DSAMP_ADDRESSW     , D._sampler_address);
   }
#elif DX11
   SamplerAnisotropic.setPS(SSI_DEFAULT);
#elif GL
 /*switch(D.texFilter()) unused
   {
      case  0: D._sampler_filter[0]=GL_LINEAR; break; // or GL_LINEAR_MIPMAP_LINEAR
      case  1: D._sampler_filter[0]=GL_LINEAR; break; // or GL_LINEAR_MIPMAP_LINEAR
      default: D._sampler_filter[0]=GL_LINEAR; break; // or GL_LINEAR_MIPMAP_LINEAR
   }

   switch(D.texFilter())
   {
      case  0: D._sampler_filter[1]=GL_NEAREST; break;
      default: D._sampler_filter[1]=GL_LINEAR ; break; // or GL_LINEAR_MIPMAP_LINEAR
   }

   D._sampler_filter[2]=(D.texMipFilter() ? GL_LINEAR : GL_NEAREST);*/

   D._sampler_address=GL_REPEAT;
#endif
}
void DisplayState::samplerShadow()
{
   sampler3D(); // we could potentially use a different sampler here with smaller anisotropic value, however quality does suffer (especially with filter=1, so don't go below 2), however performance difference is minimal, so for simplicity just use the default 3D sampler
}
void DisplayState::set2D() {                     D.clipPlane(false); D.wire(false        ); D.sampler2D();}
void DisplayState::set3D() {if(Renderer.mirror())D.clipPlane(true ); D.wire(Renderer.wire); D.sampler3D();}
/******************************************************************************/
#if GL
void DisplayState::fbo(UInt fbo)
{
   if(D._fbo!=fbo)glBindFramebuffer(GL_FRAMEBUFFER, D._fbo=fbo);
}
#endif
#if IOS
Bool DisplayState::mainFBO()C
{
   return Renderer._cur[0]==&Renderer._main || Renderer._cur_ds==&Renderer._main_ds; // check both, because only one of them can be attached
}
#endif
/******************************************************************************/
void DisplayState::setDeviceSettings()
{
   sampler2D();
   DisplayState old=T;

#if DX9
   D3D->SetRenderState(D3DRS_DITHERENABLE       , true);
   D3D->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
   D3D->SetRenderState(D3DRS_STENCILFAIL        , D3DSTENCILOP_KEEP);
   D3D->SetRenderState(D3DRS_CCW_STENCILFAIL    , D3DSTENCILOP_KEEP);
   REP(4)
   {
      D3D->SetSamplerState(D3DVERTEXTEXTURESAMPLER0+i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR  );
      D3D->SetSamplerState(D3DVERTEXTEXTURESAMPLER0+i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR  );
      D3D->SetSamplerState(D3DVERTEXTEXTURESAMPLER0+i, D3DSAMP_ADDRESSU , D3DTADDRESS_WRAP);
      D3D->SetSamplerState(D3DVERTEXTEXTURESAMPLER0+i, D3DSAMP_ADDRESSV , D3DTADDRESS_WRAP);
      D3D->SetSamplerState(D3DVERTEXTEXTURESAMPLER0+i, D3DSAMP_ADDRESSW , D3DTADDRESS_WRAP);
   }
#elif DX11
   SamplerLinearWrap .set(SSI_DEFAULT); SamplerLinearClamp.setPS(SSI_DEFAULT); // use linear wrap everywhere (needed for example in Vertex Shaders for Water), but use linear clamp for PS as it's the default 2D sampler used everywhere except 3D
   SamplerPoint      .set(SSI_POINT);
   SamplerLinearWrap .set(SSI_LINEAR_WRAP);
   SamplerLinearClamp.set(SSI_LINEAR_CLAMP);
   SamplerLinearCWW  .set(SSI_LINEAR_CWW);
   SamplerShadowMap  .set(SSI_SHADOW);
   SamplerFont       .set(SSI_FONT);
 //SPSet("AllowAlphaToCoverage", D.multiSample() || D.densityByte()==255);
#elif GL
      glEnable     (GL_DITHER);
   #if GL_ES
      glClearDepthf(REVERSE_DEPTH ? 0.0f : 1.0f);
   #else
      glClearDepth (REVERSE_DEPTH ? 0.0  : 1.0);
    //glAlphaFunc  (GL_GREATER, 0);
      glHint       (GL_LINE_SMOOTH_HINT, GL_NICEST); 
   #endif
   #if !IOS
      if(DepthReal)glEnable   (GL_DEPTH_TEST);else glDisable(GL_DEPTH_TEST);
                   glColorMask(FlagTest(Col0WriteReal, COL_WRITE_R), FlagTest(Col0WriteReal, COL_WRITE_G), FlagTest(Col0WriteReal, COL_WRITE_B), FlagTest(Col0WriteReal, COL_WRITE_A));
   #endif
#endif

  _vf=null;

  _depth      ^=1; depth     (old._depth      );
  _depth_write^=1; depthWrite(old._depth_write);
  _depth_func ^=1; depthFunc (old._depth_func );

  _line_smooth^=1; lineSmooth(!_line_smooth);
  _wire       ^=1; wire      (!_wire       );
  _cull       ^=1; cull      (!_cull       );

  _bias          ^=                  1 ; bias       ((BIAS_MODE)old._bias        );
  _alpha          =ALPHA_MODE(_alpha^1); alpha      (           old._alpha       );
  _alpha_factor.r^=                  1 ; alphaFactor(           old._alpha_factor);

  _clip_recti.set(0, -1);
  _clip      ^=1; clip     (old._clip ? &old._clip_rect : null);
  _clip_allow^=1; clipAllow(old._clip_allow);

  _col_write[0]^=1; colWrite(old._col_write[0], 0);
  _col_write[1]^=1; colWrite(old._col_write[1], 1);
  _col_write[2]^=1; colWrite(old._col_write[2], 2);
  _col_write[3]^=1; colWrite(old._col_write[3], 3);

  _viewport.set(0, -1); viewport(old._viewport);

                   stencil   ( STENCIL_ALWAYS_SET           ); // required for DX9 and GL because of 'LastStencilMode'
                   stencil   ( STENCIL_TERRAIN_TEST         );
                   stencil   ((STENCIL_MODE)old._stencil    );
  _stencil_ref^=1; stencilRef(              old._stencil_ref);

   clearShader();
}
/******************************************************************************/
void DisplayState::del()
{
#if DX11
   REPAO(BS).del();
   REPAD(i, DS)REPAD(j, DS[i])REPAD(k, DS[i][j])REPAOD(l, DS[i][j][k]).del();
   REPAD(i, RS)REPAD(j, RS[i])REPAD(k, RS[i][j])REPAD (l, RS[i][j][k])REPAOD(m, RS[i][j][k][l]).del();
#endif
}
void DisplayState::create()
{
   if(LogInit)LogN("DisplayState.create");
   clearShader();

#if DX9
   AllowLineSmooth=!Contains(D.deviceName(), "Intel", false, true); // Intel produces "fat" lines on DX9
#elif DX11
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=false;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_NONE].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable   =true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_SRC_ALPHA;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_INV_DEST_ALPHA;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_BLEND].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable   =true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_SRC_ALPHA;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_BLEND_DEC].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].  BlendOp=desc.RenderTarget[0].  BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend=desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ONE;
      desc.RenderTarget[0].DestBlend=desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_ADD].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].  BlendOp=desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_DEST_COLOR;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_DEST_ALPHA;
      desc.RenderTarget[0].DestBlend     =desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_MUL].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable   =true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_ONE;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_INV_DEST_ALPHA;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_MERGE].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_SRC_ALPHA;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_ONE ;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE ;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_ADDBLEND_KEEP].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_ONE ;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_ONE ;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE ;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_ADD_KEEP].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_SRC_ALPHA    ;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_BLEND_FACTOR ;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_BLEND_FACTOR].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_ONE;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_ONE;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_BLEND_FACTOR;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_ADD_FACTOR].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_SRC_ALPHA;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_ZERO;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ONE;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_SETBLEND_SET].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_BLEND_FACTOR;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_BLEND_FACTOR;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_BLEND_FACTOR;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_BLEND_FACTOR;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_FACTOR].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].  BlendOp     =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_INV_DEST_COLOR;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_INV_DEST_ALPHA;
      desc.RenderTarget[0].DestBlend     =desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_INVERT].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].  BlendOp     =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_BLEND_FACTOR;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_BLEND_FACTOR;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_COLOR;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_FONT].create(desc);
   }
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].  BlendOp     =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_BLEND_FACTOR;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_INV_SRC_COLOR;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ZERO;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_FONT_DEC].create(desc);
   }
   /*{
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=false;
      desc.RenderTarget[0].RenderTargetWriteMask=0;
      BS[ALPHA_NULL].create(desc);
   }*/
   /*{
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=false;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALPHA;
      BS[ALPHA_NONE_WRITE_A].create(desc);
   }*/
   /*{
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=false;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_RED|D3D11_COLOR_WRITE_ENABLE_GREEN|D3D11_COLOR_WRITE_ENABLE_BLUE;
      BS[ALPHA_NONE_WRITE_RGB].create(desc);
   }*/
   /*{
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =true;
      desc.IndependentBlendEnable=true;
      REPAO(desc.RenderTarget  ).BlendEnable=false;
      REPAO(desc.RenderTarget  ).RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
            desc.RenderTarget[0].BlendEnable=true;
            desc.RenderTarget[0].BlendOp=desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_ONE ;
            desc.RenderTarget[0].DestBlend     =D3D11_BLEND_ZERO;
            desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ZERO;
            desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ZERO;
      BS[ALPHA_NONE_COVERAGE].create(desc);
   }*/
   /*{
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =true;
      desc.IndependentBlendEnable=true;
      REPAO(desc.RenderTarget  ).BlendEnable=false;
      REPAO(desc.RenderTarget  ).RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
            desc.RenderTarget[0].BlendEnable=true;
            desc.RenderTarget[0].BlendOp=desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_ONE ;
            desc.RenderTarget[0].DestBlend     =D3D11_BLEND_ONE ;
            desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ZERO;
            desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE ;
      BS[ALPHA_ADD_COVERAGE].create(desc);
   }*/
   {
      D3D11_BLEND_DESC desc; Zero(desc);
      desc.AlphaToCoverageEnable =false;
      desc.IndependentBlendEnable=false;
      desc.RenderTarget[0].BlendEnable=true;
      desc.RenderTarget[0].BlendOp       =desc.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
      desc.RenderTarget[0]. SrcBlend     =D3D11_BLEND_ONE ;
      desc.RenderTarget[0].DestBlend     =D3D11_BLEND_ZERO;
      desc.RenderTarget[0]. SrcBlendAlpha=D3D11_BLEND_ONE ;
      desc.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE ;
      desc.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
      BS[ALPHA_NONE_ADD].create(desc);
   }

   // depth stencil state
   REPD(stencil    , STENCIL_NUM)
   REPD(depth_use  , 2)
   REPD(depth_write, 2)
   REPD(depth_func , 8)
   {
      D3D11_DEPTH_STENCIL_DESC desc; Zero(desc);
      desc.DepthEnable     =depth_use;
      desc.DepthWriteMask  =(depth_write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO);
      desc.DepthFunc       =D3D11_COMPARISON_FUNC(D3D11_COMPARISON_FIRST+depth_func);
      desc.StencilEnable   =(stencil!=STENCIL_NONE);
      desc.StencilReadMask =D3D11_DEFAULT_STENCIL_READ_MASK;
      desc.StencilWriteMask=D3D11_DEFAULT_STENCIL_WRITE_MASK;
      desc.FrontFace.StencilFailOp=desc.BackFace.StencilFailOp=D3D11_STENCIL_OP_KEEP;
      switch(stencil)
      {
         case STENCIL_ALWAYS_SET:
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_TERRAIN_TEST:
            desc.StencilReadMask      =STENCIL_REF_TERRAIN;
            desc.StencilWriteMask     =0;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_EQUAL;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_MSAA_SET:
            desc.StencilReadMask =STENCIL_REF_MSAA;
            desc.StencilWriteMask=STENCIL_REF_MSAA;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_MSAA_TEST:
            desc.StencilReadMask =STENCIL_REF_MSAA;
            desc.StencilWriteMask=0;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_EQUAL;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_EDGE_SOFT_SET:
            desc.StencilReadMask =STENCIL_REF_EDGE_SOFT;
            desc.StencilWriteMask=STENCIL_REF_EDGE_SOFT;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_EDGE_SOFT_TEST:
            desc.StencilReadMask =STENCIL_REF_EDGE_SOFT;
            desc.StencilWriteMask=0;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_EQUAL;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_WATER_SET:
            desc.StencilReadMask =STENCIL_REF_WATER;
            desc.StencilWriteMask=STENCIL_REF_WATER;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_WATER_TEST:
            desc.StencilReadMask =STENCIL_REF_WATER;
            desc.StencilWriteMask=0;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_EQUAL;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_OUTLINE_SET:
            desc.StencilReadMask =STENCIL_REF_OUTLINE;
            desc.StencilWriteMask=STENCIL_REF_OUTLINE;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_REPLACE;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;

         case STENCIL_OUTLINE_TEST:
            desc.StencilReadMask =STENCIL_REF_OUTLINE;
            desc.StencilWriteMask=0;
            desc.FrontFace.StencilFunc=desc.BackFace.StencilFunc=D3D11_COMPARISON_EQUAL;
            desc.FrontFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilPassOp     =D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
            desc. BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_KEEP;
         break;
      }

      DS[stencil][depth_use][depth_write][depth_func].create(desc);
   }

   REPD(cull, 3)
   REPD(bias, BIAS_NUM)
   REPD(line, 2)
   REPD(wire, 2)
   REPD(clip, 2)
   {
      D3D11_RASTERIZER_DESC desc; Zero(desc);
      desc.FillMode       =(wire ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID);
      desc.CullMode       =((cull==0) ? D3D11_CULL_NONE : (cull==1) ? D3D11_CULL_BACK : D3D11_CULL_FRONT);
      desc.ScissorEnable  =clip;
      desc.DepthClipEnable=true;
      if(D.shaderModel()>=SM_4_1) // in 4.1 following members affect only lines
      {
         desc.MultisampleEnable    =false;
         desc.AntialiasedLineEnable=line ;
      }else // let on 4.0 smooth lines be always disabled
      {
         desc.MultisampleEnable    =true ; // for smooth lines this should be false (but let's not bother with additional state combination only for smooth lines on 4.0)
         desc.AntialiasedLineEnable=false;
      }
      switch(bias)
      {
         case BIAS_ZERO   : desc.DepthBias=                 0; desc.SlopeScaledDepthBias=                              0; break;
         case BIAS_SHADOW : desc.DepthBias=DEPTH_BIAS_SHADOW ; desc.SlopeScaledDepthBias=SLOPE_SCALED_DEPTH_BIAS_SHADOW ; break;
         case BIAS_OVERLAY: desc.DepthBias=DEPTH_BIAS_OVERLAY; desc.SlopeScaledDepthBias=SLOPE_SCALED_DEPTH_BIAS_OVERLAY; break;
      }
      RS[cull][bias][line][wire][clip].create(desc);
   }
#endif
}
/******************************************************************************/
}
/******************************************************************************/
