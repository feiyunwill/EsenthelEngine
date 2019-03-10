PAR HP Flt VtxHeightmap;
PAR MP Flt VtxSkinning;

#define ATTR attribute
// !! must be in sync with GL_VTX_SEMANTIC !!
ATTR HP Vec4 ATTR0 ; // pos
ATTR MP Vec  ATTR9 ; // hlp
ATTR MP Vec  ATTR1 ; // nrm
ATTR MP Vec4 ATTR2 ; // tan bin
ATTR HP Vec2 ATTR3 ; // tex
ATTR HP Vec2 ATTR4 ; // tex1
ATTR HP Vec2 ATTR11; // tex2
ATTR MP Flt  ATTR10; // size
ATTR MP Vec  ATTR5 ; // bone, should be MP VecI but won't compile
ATTR MP Vec  ATTR6 ; // weight
ATTR LP Vec4 ATTR8 ; // material
ATTR LP Vec4 ATTR7 ; // color

MP Vec  vtx_nrm      () {return ATTR1 ;}
MP Vec  vtx_tan      () {return ATTR2.xyz;}
MP Flt  vtx_tanW     () {return ATTR2.w;}
HP Vec2 vtx_pos2     () {return ATTR0.xy;}
HP Vec  vtx_pos      () {return ATTR0.xyz;}
HP Vec4 vtx_pos4     () {return ATTR0 ;}
MP Vec  vtx_hlp      () {return ATTR9 ;}
HP Vec2 vtx_tex      () {return ATTR3;}
HP Vec2 vtx_texHM    () {return ATTR0.xz*Vec2(VtxHeightmap, -VtxHeightmap);}
HP Vec2 vtx_tex1     () {return ATTR4 ;}
HP Vec2 vtx_tex2     () {return ATTR11;}
MP VecI vtx_bone     () {return Bool(VtxSkinning) ? VecI(ATTR5) : VecI(0, 0, 0);}
#ifdef GL_ES // GLSL may not support "#if GL_ES" if GL_ES is not defined
MP Vec  vtx_weight   () {MP Flt w=ATTR6.x/(ATTR6.x+ATTR6.y); return Vec(w, 1.0-w, 0.0);} // use only 2 weights on OpenGL ES
#else
MP Vec  vtx_weight   () {return ATTR6;}
#endif
LP Vec4 vtx_color    () {return ATTR7    ;}
LP Vec  vtx_color3   () {return ATTR7.rgb;}
LP Vec4 vtx_material () {return ATTR8    ;}
LP Vec  vtx_material3() {return ATTR8.xyz;}
MP Flt  vtx_size     () {return ATTR10   ;}
