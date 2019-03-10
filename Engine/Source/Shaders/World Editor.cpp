/******************************************************************************/
#include "!Header.h"
/******************************************************************************/
// COLOR
/******************************************************************************/
void Color_VS
(
   VtxInput vtx,

   out VecH4 outCol:COLOR    ,
   out VecH  outNrm:TEXCOORD0,
   out Vec   outPos:TEXCOORD1,
   out Vec4  outVtx:POSITION ,

   uniform Bool vtx_col=false
)
{  
   if(vtx_col)outCol=vtx.color();

   outNrm=Normalize(TransformDir(vtx.nrm()));
   outPos=          TransformPos(vtx.pos()) ;
   outVtx=          Project     ( outPos  ) ;
}
/******************************************************************************/
void Color_PS
(
   VecH4 inCol:COLOR    ,
   VecH  inNrm:TEXCOORD0,
   Vec   inPos:TEXCOORD1,

   out DeferredSolidOutput output,

   uniform VecH color  ,
   uniform Bool vtx_col=false
)
{
   output.color   ((vtx_col ? color*inCol.rgb : color)+Highlight.rgb);
   output.glow    (0);
   output.normal  (Normalize(inNrm));
   output.specular(0);
   output.velocity(0, inPos);
}
/******************************************************************************/
// CIRCLE / SQUARE / GRID
/******************************************************************************/
BUFFER(WorldEditor)
   Flt  XZImageUse,
        XZPattern;
   Flt  XZRange,
        XZSoft,
        XZAngle,
        XZPatternScale;
   Vec2 XZPos;
   VecH XZCol;
BUFFER_END

Image XZImage;
/******************************************************************************/
struct VS_PS
{
   Vec  pos  :TEXCOORD0,
        nrm  :TEXCOORD1;
   Vec2 pos2D:TEXCOORD2;
   Vec4 vtx  :POSITION ;
};
struct VS_PS_NOVTX
{
   Vec  pos  :TEXCOORD0,
        nrm  :TEXCOORD1;
   Vec2 pos2D:TEXCOORD2;
};
/******************************************************************************/
VS_PS FX_VS
(
   VtxInput vtx
)
{
   VS_PS O;
   O.pos=          TransformPos(vtx.pos()) ; O.pos2D=Transform(O.pos, CamMatrix).xz;
   O.nrm=Normalize(TransformDir(vtx.nrm()));
   O.vtx=          Project     (  O.pos  ) ;
   return O;
}
/******************************************************************************/
Vec4 Circle_PS
(
   VS_PS I
):COLOR
{
   Vec2 cos_sin; CosSin(cos_sin.x, cos_sin.y, XZAngle);
   Vec2 d=I.pos2D-XZPos; d=Rotate(d, cos_sin); d/=XZRange;

   Flt b;
   b=Length(d);
   b=Sat   ((b-(1-XZSoft))/XZSoft);
   b=BlendSmoothCube(b);
   if(XZImageUse)b*=Tex(XZImage, (XZPattern ? I.pos2D*XZPatternScale : d*0.5f+0.5f)*Vec2(1,-1)).r;

   return Vec4(XZCol*b, 0);
}
/******************************************************************************/
Vec4 Square_PS
(
   VS_PS I
):COLOR
{
   Vec2 cos_sin; CosSin(cos_sin.x, cos_sin.y, XZAngle);
   Vec2 d=I.pos2D-XZPos; d=Rotate(d, cos_sin); d/=XZRange;

   Flt b;
   b=Max(Abs(d));
   b=Sat((b-(1-XZSoft))/XZSoft);
   b=BlendSmoothCube(b);
   if(XZImageUse)b*=Tex(XZImage, (XZPattern ? I.pos2D*XZPatternScale : d*0.5f+0.5f)*Vec2(1, -1)).r;

   return Vec4(XZCol*b, 0);
}
/******************************************************************************/
Vec4 Grid_PS
(
   VS_PS_NOVTX I
):COLOR
{
   Vec2 pos  =I.pos2D/XZRange;
   Vec2 xz   =Sat((Abs(Frac(pos)-0.5f)-0.5f)/XZSoft+1);
   Flt  alpha=Max(xz);
   Flt  dd=Max(Vec4(Abs(ddx(pos)), Abs(ddy(pos)))); alpha*=LerpRS(0.2f, 0.1f, dd);
   return Vec4(XZCol*alpha, 0);
}
/******************************************************************************/
// HULL / DOMAIN
/******************************************************************************/
#if MODEL>=SM_4
HSData HSConstant(InputPatch<VS_PS_NOVTX,3> I) {return GetHSData(I[0].pos, I[1].pos, I[2].pos, I[0].nrm, I[1].nrm, I[2].nrm);}
[maxtessfactor(5.0)]
[domain("tri")]
[partitioning("fractional_odd")] // use 'odd' because it supports range from 1.0 ('even' supports range from 2.0)
[outputtopology("triangle_cw")]
[patchconstantfunc("HSConstant")]
[outputcontrolpoints(3)]
VS_PS_NOVTX HS
(
   InputPatch<VS_PS_NOVTX,3> I, UInt cp_id:SV_OutputControlPointID
)
{
   VS_PS_NOVTX O;
   O.pos  =I[cp_id].pos;
   O.nrm  =I[cp_id].nrm;
   O.pos2D=I[cp_id].pos2D;
   return O;
}
/******************************************************************************/
[domain("tri")]
VS_PS DS
(
   HSData hs_data, const OutputPatch<VS_PS_NOVTX,3> I, Vec B:SV_DomainLocation
)
{
   VS_PS O;

   O.pos2D=I[0].pos2D*B.z + I[1].pos2D*B.x + I[2].pos2D*B.y;

   SetDSPosNrm(O.pos, O.nrm, I[0].pos, I[1].pos, I[2].pos, I[0].nrm, I[1].nrm, I[2].nrm, B, hs_data, false, 0);
   O.vtx=Project(O.pos);

   return O;
}
#endif
/******************************************************************************/
TECHNIQUE(WhiteVtx, Color_VS(true), Color_PS(Vec(1, 1, 1), true ));
TECHNIQUE(White   , Color_VS(    ), Color_PS(Vec(1, 1, 1), false));
TECHNIQUE(Green   , Color_VS(    ), Color_PS(Vec(0, 1, 0), false));
TECHNIQUE(Yellow  , Color_VS(    ), Color_PS(Vec(1, 1, 0), false));
TECHNIQUE(Red     , Color_VS(    ), Color_PS(Vec(1, 0, 0), false));

TECHNIQUE            (Circle <bool ForceHP=true;>, FX_VS(), Circle_PS());
TECHNIQUE            (Square <bool ForceHP=true;>, FX_VS(), Square_PS());
TECHNIQUE            (Grid   <bool ForceHP=true;>, FX_VS(),   Grid_PS());
TECHNIQUE_TESSELATION(CircleT                    , FX_VS(), Circle_PS(), HS(), DS());
TECHNIQUE_TESSELATION(SquareT                    , FX_VS(), Square_PS(), HS(), DS());
TECHNIQUE_TESSELATION(GridT                      , FX_VS(),   Grid_PS(), HS(), DS());
/******************************************************************************/
#if 0
@GROUP "Circle"

   @SHARED
      #include "Glsl.h"

      VAR HP Vec2 IO_pos2D;
   @SHARED_END

   @VS
      #include "Glsl VS.h"
      #include "Glsl VS 3D.h"

      void main()
      {
         HP Vec pos=TransformPos(vtx_pos());
         IO_pos2D=Transform(pos, CamMatrix).xz;
         O_vtx=Project(pos);
      }
   @VS_END

   @PS
      #include "Glsl PS.h"

      void main()
      {
   Vec2 cos_sin; CosSin(cos_sin.x, cos_sin.y, XZAngle);
   Vec2 d=I.pos2D-XZPos; d=Rotate(d, cos_sin); d/=XZRange;

   Flt b;
   b=Length(d);
   b=Sat   ((b-(1-XZSoft))/XZSoft);
   b=BlendSmoothCube(b);
   if(XZImageUse)b*=Tex(XZImage, (XZPattern ? I.pos2D*XZPatternScale : d*0.5f+0.5f)*Vec2(1,-1)).r;

   return Vec4(XZCol*b, 0);

         gl_FragColor.rgb=col; // set 'gl_FragColor' at end since it's MP
         gl_FragColor.a  =((per_pixel!=0) ? glow : 0.0);
      }
   @PS_END

@GROUP_END
#endif
/******************************************************************************/
