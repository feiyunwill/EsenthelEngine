/******************************************************************************/
enum SEL_FLAG // Selection Flag
{
   SEL_A =0x1, // if A  present
   SEL_B =0x2, // if B  present
   SEL_AB=0x4, // if AB present

   SEL_OR  =SEL_A|SEL_B|SEL_AB, // A|B
   SEL_XOR =SEL_A|SEL_B       , // A^B
   SEL_AND =            SEL_AB, // A&B
   SEL_ADD =SEL_A|SEL_B|SEL_AB, // A+B
   SEL_SUB =SEL_A             , // A-B
   SEL_SUBR=      SEL_B       , // B-A
   SEL_MUL =            SEL_AB, // A*B
};
/******************************************************************************/
#if EE_PRIVATE
enum SIDE_TYPE
{
   SIDE_NONE=-1,
   SIDE_L   = 0,
   SIDE_R   = 1,
};
enum
{
   TYPE_A=0,
   TYPE_B=1,
};
enum SOLID_FLAG
{
   SOLID_AL =0x01,
   SOLID_AR =0x02,
   SOLID_BL =0x04,
   SOLID_BR =0x08,
   SOLID_NAL=0x10,
   SOLID_NAR=0x20,
   SOLID_NBL=0x40,
   SOLID_NBR=0x80,

   SOLID_A  =SOLID_AL|SOLID_AR|                  SOLID_NAL|SOLID_NAR                    ,
   SOLID_B  =                  SOLID_BL|SOLID_BR|                    SOLID_NBL|SOLID_NBR,
   SOLID_L  =SOLID_AL|         SOLID_BL|         SOLID_NAL|          SOLID_NBL          ,
   SOLID_R  =         SOLID_AR|         SOLID_BR|          SOLID_NAR|          SOLID_NBR,
   SOLID_YES=SOLID_AL|SOLID_AR|SOLID_BL|SOLID_BR                                        ,
   SOLID_NO =                                    SOLID_NAL|SOLID_NAR|SOLID_NBL|SOLID_NBR,
   SOLID_ALL=SOLID_AL|SOLID_AR|SOLID_BL|SOLID_BR|SOLID_NAL|SOLID_NAR|SOLID_NBL|SOLID_NBR,
};
/******************************************************************************/
inline UInt   SolidT   (           Bool type                    ) {return (type==TYPE_A) ? SOLID_A   : SOLID_B ;}
inline UInt   SolidS   (                      Bool side         ) {return (side==SIDE_L) ? SOLID_L   : SOLID_R ;}
inline UInt   SolidI   (                                 Bool is) {return  is            ? SOLID_YES : SOLID_NO;}
inline UInt   SolidTI  (           Bool type,            Bool is) {return      SolidT(type)&             SolidI(is);}
inline UInt   SolidTSI (           Bool type, Bool side, Bool is) {return      SolidT(type)&SolidS(side)&SolidI(is);}
inline UInt   SolidT   (UInt flag, Bool type                    ) {return flag&SolidT(type)             ;}
inline UInt   SolidS   (UInt flag,            Bool side         ) {return flag&             SolidS(side);}
inline UInt   SolidTS  (UInt flag, Bool type, Bool side         ) {return flag&SolidT(type)&SolidS(side);}
inline Bool IsSolidTS  (UInt flag, Bool type, Bool side         ) {return (SolidTS(flag, type, side)&SOLID_YES)!=0;}
       UInt   SolidSwap(UInt flag);
#endif
/******************************************************************************/
void Csg(MeshBase &a, C MeshBase &b, UInt sel, MeshBase *dest=null, Bool detect_self_intersections=false, Flt weld_pos_eps=0.0001f); // perform a CSG operation on 'a b' meshes, 'sel'=SEL_FLAG, 'dest'=output mesh (if set to null then it will be set to 'a'), 'detect_self_intersections'=if detect self intestections between 'a' mesh faces, and self intersections between 'b' mesh faces (this will make Csg perform slower but correctly handle situations where 'a' mesh faces intersect each other or 'b' mesh faces intersect each other), 'weld_pos_eps'=epsilon used for final vertex position welding, if you're getting incorrect results you can try calling first 'removeDegenerateFaces' on meshes
void Csg(MeshLod  &a, C MeshLod  &b, UInt sel, MeshLod  *dest=null, Bool detect_self_intersections=false, Flt weld_pos_eps=0.0001f); // perform a CSG operation on 'a b' meshes, 'sel'=SEL_FLAG, 'dest'=output mesh (if set to null then it will be set to 'a'), 'detect_self_intersections'=if detect self intestections between 'a' mesh faces, and self intersections between 'b' mesh faces (this will make Csg perform slower but correctly handle situations where 'a' mesh faces intersect each other or 'b' mesh faces intersect each other), 'weld_pos_eps'=epsilon used for final vertex position welding, if you're getting incorrect results you can try calling first 'removeDegenerateFaces' on meshes

void  ClipMesh     (C MeshBase &src, C Matrix *matrix, MeshBase &dest         ,                      C Plane *clip_plane, Int clip_planes,                          UInt flag_and=~0, Flt weld_pos_eps=EPS); // generate 'src' mesh transformed by 'matrix' and then clipped by 'clip_planes' (if the 'matrix' is null then the mesh is not transformed), result is stored in 'dest'                             , 'weld_pos_eps'=epsilon used for final vertex position welding, 'flag_and'=elements to include in 'dest'   creation (MESH_BASE_FLAG)
void  ClipMesh     (C Mesh     &src, C Matrix *matrix, Mesh     &dest         ,                      C Plane *clip_plane, Int clip_planes,                          UInt flag_and=~0, Flt weld_pos_eps=EPS); // generate 'src' mesh transformed by 'matrix' and then clipped by 'clip_planes' (if the 'matrix' is null then the mesh is not transformed), result is stored in 'dest'                             , 'weld_pos_eps'=epsilon used for final vertex position welding, 'flag_and'=elements to include in 'dest'   creation (MESH_BASE_FLAG)
void SplitMesh     (C Mesh     &src, C Matrix *matrix, Mesh     &dest_positive, Mesh &dest_negative, C Plane &clip_plane,                                           UInt flag_and=~0, Flt weld_pos_eps=EPS); // generate 'src' mesh transformed by 'matrix' and then split   by 'clip_plane'  (if the 'matrix' is null then the mesh is not transformed), result is stored in 'dest_positive' and 'dest_negative', 'weld_pos_eps'=epsilon used for final vertex position welding, 'flag_and'=elements to include in 'dest_*' creation (MESH_BASE_FLAG)
void SplitMeshSolid(C Mesh     &src, C Matrix *matrix, Mesh     &dest_positive, Mesh &dest_negative, C Plane &clip_plane, C MaterialPtr &material, Flt tex_scale=1, UInt flag_and=~0, Flt weld_pos_eps=EPS); // generate 'src' mesh transformed by 'matrix' and then split   by 'clip_plane'  (if the 'matrix' is null then the mesh is not transformed), result is stored in 'dest_positive' and 'dest_negative', 'weld_pos_eps'=epsilon used for final vertex position welding, 'flag_and'=elements to include in 'dest_*' creation (MESH_BASE_FLAG), 'material'=material used to fill the "solid inside" created because of the splitting, 'tex_scale'=texture coordinates scale of the "solid inside" part
/******************************************************************************/
