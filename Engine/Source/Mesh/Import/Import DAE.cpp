/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CONVERT_BIND_POSE 1 // do not undefine
/******************************************************************************

   TODO:
      animations Lerp -> Lerp4
      process "ScaleAxisR" and "ScaleAxis" keyframes for non-baked matrixes

   INFO:
      if when using CONVERT_BIND_POSE some vertexes go crazy,
         this means that they have incorrect skinning (for example face vertexes have partially set body bone),
         matrixes with bad sign on "x" direction can also affect that result

      OpenCollada doesn't contain any information about animation length,
         so it must be calculated from the max value of keyframe time positions.

      Node is imported as Bone only if at least one of following conditions is met:
         -node has type="JOINT" in its declaration
         -node has any skinning (vertexes attached to the node)
         -node has animation keyframes and there is a (bone or mesh) in children

/******************************************************************************/
// STRUCT
/******************************************************************************/
enum SEMANTIC : Byte
{
   NONE    ,
   INPUT   ,
   OUTPUT  ,
   INTERPOLATION,
   VERTEX  ,
   POSITION,
   NORMAL  ,
   TEXCOORD,
   JOINT   ,
   WEIGHT  ,
   INV_BIND_MATRIX,
};
/******************************************************************************/
struct DAE
{
   struct Effect  ;
   struct Mesh    ;
   struct Skin    ;
   struct Node    ;
   struct Source  ;
   struct Input   ;
   struct Vertices;

   struct Sampler
   {
      Str     id;
      Source *input, *output;

      void import(XmlNode &node, Memb<Source> &source);

      Sampler() {input=output=null;}
   };
   struct Channel
   {
      Str     target;
      Source *time, *transform, *move, *move_x, *move_y, *move_z, *rot_z, *rot_y, *rot_x, *scale_axis_r, *scale, *scale_axis, *alpha;
      Node   *node;

      void import  (XmlNode &node, Memb<Sampler> &sampler, DAE &dae);
      void linkNode(DAE &dae);
      Bool anyDifferent()C; // if any of the keyframe is different from each other

      Channel() {time=transform=move=move_x=move_y=move_z=rot_z=rot_y=rot_x=scale_axis_r=scale=scale_axis=alpha=null; node=null;}
   };
   struct Animation
   {
      Str             id;
      Memb<Source   > source;
      Memb<Sampler  > sampler;
      Memb<Channel  > channel;
      Memb<Animation> animation;

      void import  (XmlNode &node, DAE &dae);
      void linkNode(DAE &dae);
   };
   struct Image
   {
      Str id, name, texture;

      void import(XmlNode &node);
   };
   struct Material
   {
      Str     id, name, fx;
      Effect *effect;

      void import    (XmlNode &node);
      void linkEffect(DAE     &dae );

      Material() {effect=null;}
   };
   struct NewParam
   {
      Str sid, image_id;

      void import(XmlNode &node, Memb<NewParam> &param, DAE &dae);
   };
   struct Effect
   {
      Bool           double_sided;
      Str            id, name, color_map_image_id, alpha_map_image_id, specular_map_image_id, bump_map_image_id;
      Flt            shininess, spec_level;
      Vec            ambient, color, specular;
      Memb<NewParam> param;

      void import(XmlNode &node, DAE &dae);

      Effect() {double_sided=false; ambient=0; color=1; specular=0; shininess=0; spec_level=-1;}
   };
   struct Source
   {
      Str        id;
      Int        stride;
      Memc<Flt > float_array;
      Memc<Str8>  name_array;

      CChar8* getName  (Int i) {i*=stride; return  InRange(i,  name_array)                               ?       name_array[i+0]()                                                      : null;}
      Flt     getFloat (Int i) {i*=stride; return  InRange(i, float_array)                               ?      float_array[i+0]                                                        : 0;}
      Vec2    getVec2  (Int i) {i*=stride; return (InRange(i, float_array) && InRange(i+1, float_array)) ? Vec2(float_array[i+0], float_array[i+1]                                    ) : 0;}
      Vec     getVec   (Int i) {i*=stride; return (InRange(i, float_array) && InRange(i+2, float_array)) ? Vec (float_array[i+0], float_array[i+1], float_array[i+2]                  ) : VecZero;}
      Vec4    getVec4  (Int i) {i*=stride; return (InRange(i, float_array) && InRange(i+3, float_array)) ? Vec4(float_array[i+0], float_array[i+1], float_array[i+2], float_array[i+3]) : 0;}
      Matrix  getMatrix(Int i)
      {
         i*=stride;
         if(InRange(i   , float_array)
         && InRange(i+11, float_array))
         {
            Matrix m;
            m.x  .set(float_array[i+0], float_array[i+4], float_array[i+ 8]);
            m.y  .set(float_array[i+1], float_array[i+5], float_array[i+ 9]);
            m.z  .set(float_array[i+2], float_array[i+6], float_array[i+10]);
            m.pos.set(float_array[i+3], float_array[i+7], float_array[i+11]);
            return m;
         }
         return MatrixIdentity;
      }

      void import(XmlNode &node);

      Source() {stride=1;}
   };
   struct Input
   {
      SEMANTIC  semantic;
      Source   *source;
      Vertices *vertices;
      Int       offset, set;

      void import(XmlNode &node, Memb<Source> &source, Mesh *mesh=null);

      Input() {semantic=NONE; source=null; vertices=null; offset=0; set=0;}
   };
   struct Vertices
   {
      Str         id;
      Memb<Input> inputs;

      void import(XmlNode &node, Mesh &mesh);
   };
   struct Triangles
   {
      Str         material_symbol;
      Memb<Input> inputs;
      Memc<Int  > p;

      void import(XmlNode &node, Mesh &mesh, DAE &dae);
   };
   struct PolyList
   {
      Str         material_symbol;
      Memb<Input> inputs;
      Memc<Int>   vcount, p;

      void import(XmlNode &node, Mesh &mesh, DAE &dae);
   };
   struct Polygons
   {          
      Str               material_symbol;
      Memb< Input     > inputs;
      Memb< Memc<Int> > p;

      void import(XmlNode &node, Mesh &mesh, DAE &dae);
   };
   struct Mesh
   {
      Memb<Source   > source   ;
      Memb<Vertices > vertices ;
      Memb<Triangles> triangles;
      Memb<PolyList > poly_list;
      Memb<Polygons > polygons ;

      void import(XmlNode &node, DAE &dae);
   };
   struct Geometry
   {
      Str        id  ,
                 name;
      Memb<Mesh> mesh;
      Skin      *skin;

      void import(XmlNode &node, DAE &dae);

      Geometry() {skin=null;}
   };
   struct Joint
   {
      Memb<Input> inputs;

      void import(XmlNode &node, Memb<Source> &source);
   };
   struct VertexWeights
   {
      Memb<Input> inputs;
      Memc<Int  > vcount;
      Memc<Int  > v;
      Input      *joint, *weight;

      void import(XmlNode &node, Memb<Source> &source);

      VertexWeights() {joint=weight=null;}
   };
   struct Skin // exists as 1 element in 'Controller'
   {
      Geometry    *geometry;
      Str          geometry_name;
      Matrix       bind_shape_matrix;
      Source      *name, *matrix;
      Memb<Source> source;
      Memb<Joint > joint ;
      Memc<Byte  > bone_remap; // remap from local 'joint' set to global index of bones in target skeleton
      Memb<VertexWeights> vtx_wgt;

      void import(XmlNode &node, DAE &dae);

      Skin() {geometry=null; bind_shape_matrix.identity(); name=matrix=null;}
   };
   struct Controller // exists as 1 element for each 'Geometry', contains set of bones (with matrixes) and vertex skinning for that bones (here are listed only bones which may have skinning, dummy/helpers/bones without skinning aren't listed)
   {
      Str        id;
      Memb<Skin> skin;

      void import(XmlNode &node, DAE &dae);
      void linkSkin(DAE &dae);
   };
   struct InstanceMaterial
   {
      Str material_symbol, material_id;
   };
   struct InstanceGeometry
   {
      Str geometry_id;
      Memb<InstanceMaterial> instance_material;

      void import(XmlNode &node);
   };
   struct InstanceController
   {
      Str controller_id;
      Memb<InstanceMaterial> instance_material;

      void import(XmlNode &node);
   };
   struct Node
   {
      Str                      id, name, sid;
      Bool                     bone;
      Int                      bone_index; // index of this bone in 'DAE.bones' container
      Vec4                     color;
      Matrix                   local_matrix, anim_matrix, world_matrix;
      Node                    *parent  ;
      Memb<Node              > nodes   ; // children
      Memb<Channel          *> channels; // animation channels
      Memb<InstanceGeometry  > instance_geometry;
      Memb<InstanceController> instance_controller;

      Node() {bone=false; bone_index=-1; color=1; local_matrix.identity(); anim_matrix.identity(); parent=null;}

      // create
      void create(::Mesh &mesh, MemPtr<Int> part_material_index, ::Skeleton &skeleton, DAE &dae);

      // get
      Node* findNode(CChar *name)
      {
         if(Equal(id, name))return this;
         REPA(nodes)if(Node *node=nodes[i].findNode(name))return node;
         return null;
      }
      Bool hasAnim()
      {
         if(channels.elms())
         {
            REPA(channels)if(channels[i]->anyDifferent())return true; // check if any keyframes are different from each other
            animate(0); // animate to first frame
            if(!Equal(local_matrix, anim_matrix, EPS_ANIM_ANGLE, EPS_ANIM_POS))return true; // check if first frame is animated (not the same as local matrix)
         }
         return false;
      }

      // set
      void linkBone      (DAE &dae);
      void linkSkin      (DAE &dae);
      void setWorldMatrix(        ) {world_matrix=local_matrix; if(parent)world_matrix*=parent->world_matrix; REPAO(nodes).setWorldMatrix();}
      void defaultPose   (        ) { anim_matrix=local_matrix;                                               REPAO(nodes).defaultPose   ();}
      
      void setBoneNodesFromSkin(DAE &dae);
      Bool setBoneNodesFromAnim(DAE &dae);

      void adjustBoneIndex(Int bone_index)
      {
         if(bone_index>=0xFF)bone_index=-1;
          T.bone_index=bone_index;
         if(bone_index<0)bone=false; // if we're clearing bone index, then it means we don't want this as a bone
      }

      // io
      void import(XmlNode &node, Node *parent);

      // animate
      void animate(Flt t);

   #if DEBUG
      // draw
      void draw(C Matrix &parent, Bool anim, Flt scale)
      {
         Matrix m=(anim ? anim_matrix : local_matrix)*parent;

         if(!Kb.ctrl() &&  bone)Matrix(m).scaleOrn(scale).draw();
         if( Kb.ctrl() && !bone)Matrix(m).scaleOrn(scale).draw();

         REPAO(nodes).draw(m, anim, scale);
      }
   #endif
   };
   struct VisualScene
   {
      Str        id, name;
      Memb<Node> nodes;

      void import(XmlNode &node, DAE &dae);
   #if DEBUG
      void draw  (Bool anim, Flt scale) {REPAO(nodes).draw(MatrixIdentity, anim, scale);}
   #endif
   };
   struct Skinning
   {
      VecB4 bone, blend;
   };
   struct Vtx
   {
      Vec   pos, nrm;
      Vec2  tex[4];
      VecB4 matrix, blend;
   };

   Str               version;
   Int               up_axis;
   Flt               scale, duration, force_duration, fps;
   Memb<Animation  > animations;
   Memb<Image      > images;
   Memb<Material   > materials;
   Memb<Effect     > effects;
   Memb<Geometry   > geometries;
   Memb<Controller > controllers;
   Memb<VisualScene> visual_scenes;
   Memc<Node*      > bones; // manually created helper, lists bones (node of joint type) in their order of appearance

   DAE() {up_axis=-1; scale=1; duration=0; force_duration=-1; fps=0;}

   // get
   Int         findMaterial  (CChar *name);
   Int         findMaterial  (CChar *symbol, Memb<InstanceMaterial> &im);
   Image*      findImage     (CChar *name);
   Geometry*   findGeometry  (CChar *name);
   Controller* findController(CChar *name);
   Node*       findNode      (CChar *name);

   // import
   void loadAsset              (XmlNode &node);
   void loadLibraryAnimations  (XmlNode &node);
   void loadLibraryImages      (XmlNode &node);
   void loadLibraryMaterials   (XmlNode &node);
   void loadLibraryEffects     (XmlNode &node);
   void loadLibraryGeometries  (XmlNode &node);
   void loadLibraryControllers (XmlNode &node);
   void loadLibraryVisualScenes(XmlNode &node);

   void setBoneNodesFromSkin();
   void setBoneNodesFromAnim();

   // create
   void create(     ::Mesh       &mesh     ,   MemPtr<Int> part_material_index, ::Skeleton &skeleton, Geometry &geometry, Memb<InstanceMaterial> &im, Node &node);
   void create(     ::Mesh       &mesh     ,   MemPtr<Int> part_material_index, ::Skeleton &skeleton);
   void create(     ::Skeleton   &skeleton ,   XAnimation *animation);
   void create(       XAnimation &animation, ::Skeleton   &skeleton );
   void create(MemPtr<XMaterial>  materials,   Str         path     );

#if DEBUG
   // animate and draw
   void draw(Bool anim, Flt scale) {REPAO(visual_scenes).draw(anim, scale);}
#endif
};
/******************************************************************************/
// HELPER FUNCTIONS
/******************************************************************************/
static Int Safe(Memc<Int> &m, Int i) {return InRange(i, m) ? m[i] : -1;}

static void Import(Memc<Str8>  &texts , XmlNode &node) {texts .setNum(node.data.elms()); REPAO(texts )=        node.data[i] ;}
static void Import(Memc<Int>   &values, XmlNode &node) {values.setNum(node.data.elms()); REPAO(values)=TextInt(node.data[i]);}
static void Import(Memc<Flt>   &values, XmlNode &node) {values.setNum(node.data.elms()); REPAO(values)=TextFlt(node.data[i]);}
static void Import(     Bool   &value , XmlNode &node) {value       =(InRange(0, node.data) ? TextBool(node.data[0]) : false);}
static void Import(     Str    &text  , XmlNode &node) {text        =(InRange(0, node.data) ?          node.data[0]  : S);}
static void Import(     Flt    &value , XmlNode &node) {value       =(InRange(0, node.data) ? TextFlt (node.data[0]) : 0);}
static void Import(     Vec    &vec   , XmlNode &node) {REPAO(vec.c)=(InRange(i, node.data) ? TextFlt (node.data[i]) : 0);}
static void Import(     Vec4   &vec   , XmlNode &node) {REPAO(vec.c)=(InRange(i, node.data) ? TextFlt (node.data[i]) : 0);}
static void Import(     Matrix &matrix, XmlNode &node)
{
   matrix.identity();
   REP(Min(node.data.elms(), 12))
   {
      Flt f=TextFlt(node.data[i]);
      switch(i)
      {
         case  0: matrix.x  .x=f; break;
         case  4: matrix.x  .y=f; break;
         case  8: matrix.x  .z=f; break;

         case  1: matrix.y  .x=f; break;
         case  5: matrix.y  .y=f; break;
         case  9: matrix.y  .z=f; break;

         case  2: matrix.z  .x=f; break;
         case  6: matrix.z  .y=f; break;
         case 10: matrix.z  .z=f; break;

         case  3: matrix.pos.x=f; break;
         case  7: matrix.pos.y=f; break;
         case 11: matrix.pos.z=f; break;
      }
   }
}
/******************************************************************************/
static DAE::Source* FindSource(Memb<DAE::Source> &source, CChar *name)
{
   REPA(source)if(Equal(source[i].id, name))return &source[i];
   return null;
}
static DAE::Sampler* FindSampler(Memb<DAE::Sampler> &sampler, CChar *name)
{
   REPA(sampler)if(Equal(sampler[i].id, name))return &sampler[i];
   return null;
}
/******************************************************************************/
// GET
/******************************************************************************/
Int DAE::findMaterial(CChar *name)
{
   REPA(materials)if(Equal(materials[i].id, name))return i;
   return -1;
}
Int DAE::findMaterial(CChar *symbol, Memb<InstanceMaterial> &im)
{
   REPA(im)if(Equal(im[i].material_symbol, symbol))
   {
      Int m=findMaterial(im[i].material_id);
      if( m>=0)return m;
   }
   return -1;
}
DAE::Image* DAE::findImage(CChar *name)
{
   REPA(images)if(Equal(images[i].id, name))return &images[i];
   return null;
}
DAE::Geometry* DAE::findGeometry(CChar *name)
{
   REPA(geometries)if(Equal(geometries[i].id, name))return &geometries[i];
   return null;
}
DAE::Controller* DAE::findController(CChar *name)
{
   REPA(controllers)if(Equal(controllers[i].id, name))return &controllers[i];
   return null;
}
DAE::Node* DAE::findNode(CChar *name)
{
   REPA(visual_scenes)REPAD(j, visual_scenes[i].nodes)if(Node *node=visual_scenes[i].nodes[j].findNode(name))return node;
   return null;
}
/******************************************************************************/
// IMPORT
/******************************************************************************/
void DAE::Sampler::import(XmlNode &node, Memb<Source> &source)
{
   if(XmlParam *id=node.findParam("id"))T.id=id->asText();

   FREPA(node.nodes)if(node.nodes[i].name=="input")
   {
      Input input; input.import(node.nodes[i], source);
      if(input.source)switch(input.semantic)
      {
         case  INPUT: T.input =input.source; break;
         case OUTPUT: T.output=input.source; break;
      }
   }
}
void DAE::Channel::import(XmlNode &node, Memb<Sampler> &sampler, DAE &dae)
{
   Source *output=null;
   if(XmlParam *p=node.findParam("source"))if(Sampler *s=FindSampler(sampler, SkipStart(p->value, '#')))
   {
      T.time  =s-> input;
        output=s->output;
   }
   if(XmlParam *p=node.findParam("target"))
   {
        target=GetPath(p->value);
      Str type=GetBase(p->value);

      if(Equal(type, "transform"    ) || Equal(type, "matrix"       )                                  )transform   =output;else
      if(Equal(type, "translation"  )                                                                  )move        =output;else
      if(Equal(type, "translation.X") || Equal(type, "translate.X"  ) || Equal(type, "transform(3)(0)"))move_x      =output;else
      if(Equal(type, "translation.Y") || Equal(type, "translate.Y"  ) || Equal(type, "transform(3)(1)"))move_y      =output;else
      if(Equal(type, "translation.Z") || Equal(type, "translate.Z"  ) || Equal(type, "transform(3)(2)"))move_z      =output;else
      if(Equal(type, "RotZ.ANGLE"   ) || Equal(type, "RotateZ.ANGLE") || Equal(type, "rotationZ.ANGLE"))rot_z       =output;else
      if(Equal(type, "RotY.ANGLE"   ) || Equal(type, "RotateY.ANGLE") || Equal(type, "rotationY.ANGLE"))rot_y       =output;else
      if(Equal(type, "RotX.ANGLE"   ) || Equal(type, "RotateX.ANGLE") || Equal(type, "rotationX.ANGLE"))rot_x       =output;else
      if(Equal(type, "ScaleAxisR"   )                                                                  )scale_axis_r=output;else
      if(Equal(type, "scale"        )                                                                  )scale       =output;else
      if(Equal(type, "ScaleAxis"    )                                                                  )scale_axis  =output;else
      if(Equal(type, "transparency" )                                                                  )alpha       =output;
   }

   if(time && time->float_array.elms())MAX(dae.duration, time->float_array.last());
#if 0 && DEBUG
   if(time)FREPA(time->float_array)Log(S+time->getFloat(i)+" "); LogN();
#endif
}
void DAE::Animation::import(XmlNode &node, DAE &dae)
{
   if(XmlParam *id=node.findParam("id"))T.id=id->asText();

   FREPA(node.nodes)
   {
      XmlNode &n=node.nodes[i];
      if(n.name=="source"   )source   .New().import(n              );else
      if(n.name=="sampler"  )sampler  .New().import(n, source      );else
      if(n.name=="channel"  )channel  .New().import(n, sampler, dae);else
      if(n.name=="animation")animation.New().import(n,          dae);
   }
}
void DAE::Image::import(XmlNode &node)
{
   if(XmlParam *id  =node.findParam("id"  ))T.id  =id  ->asText();
   if(XmlParam *name=node.findParam("name"))T.name=name->asText();

   if(XmlNode *n=node.findNode("init_from"))if(n->data.elms())texture=SkipStartPath(SkipStartPath(SkipStartPath(Replace(n->data[0], "%20", " "), "file:///"), "file://"), "./");
}
void DAE::Material::import(XmlNode &node)
{
   if(XmlParam *id  =node.findParam("id"  ))T.id  =id  ->asText();
   if(XmlParam *name=node.findParam("name"))T.name=name->asText();

   if(XmlNode *n=node.findNode("instance_effect"))if(XmlParam *p=n->findParam("url"))fx=SkipStart(p->value, '#');
}
void DAE::NewParam::import(XmlNode &node, Memb<NewParam> &param, DAE &dae)
{
   if(XmlParam *sid=node.findParam("sid"))T.sid=sid->asText();

   if(XmlNode *n=node.findNode("surface"  ))if(XmlNode *i=n->findNode("init_from"))Import(image_id, *i);
   if(XmlNode *n=node.findNode("sampler2D"))if(XmlNode *s=n->findNode("source"   ))if(s->data.elms())REPA(param)if(param[i].sid==s->data[0]){image_id=param[i].image_id; break;}
}
void DAE::Effect::import(XmlNode &node, DAE &dae)
{
   if(XmlParam *id  =node.findParam("id"  ))T.id  =id  ->asText();
   if(XmlParam *name=node.findParam("name"))T.name=name->asText();

   if(XmlNode *n=node.findNode("profile_COMMON"))
   {
      FREPA(n->nodes)if(n->nodes[i].name=="newparam")param.New().import(n->nodes[i], param, dae);
      if(XmlNode *technique=n->findNode("technique"))
      {
         XmlNode *sub=technique->findNode("blinn"); if(!sub)sub=technique->findNode("phong");
         if(sub)
         {
            if(XmlNode *ambient  =sub->findNode("ambient"  ))if(XmlNode *color=ambient  ->findNode("color"))Import(T.ambient  , *color);
            if(XmlNode *specular =sub->findNode("specular" ))if(XmlNode *color=specular ->findNode("color"))Import(T.specular , *color);
            if(XmlNode *shininess=sub->findNode("shininess"))if(XmlNode *n    =shininess->findNode("float"))Import(T.shininess, *n);
            if(XmlNode *diffuse  =sub->findNode("diffuse"  ))
            {
               if(XmlNode *color  =diffuse->findNode("color"  ))Import(T.color, *color);
               if(XmlNode *texture=diffuse->findNode("texture"))if(XmlParam *t=texture->findParam("texture"))
               {
                  color_map_image_id=t->value;
                  REPA(param)if(param[i].sid==t->value){color_map_image_id=param[i].image_id; break;}
               }
            }
            if(XmlNode *transparent=sub->findNode("transparent"))
            {
               if(XmlNode *texture=transparent->findNode("texture"))if(XmlParam *t=texture->findParam("texture"))
               {
                  alpha_map_image_id=t->value;
                  REPA(param)if(param[i].sid==t->value){alpha_map_image_id=param[i].image_id; break;}
               }
            }
         }
         if(XmlNode *extra=technique->findNode("extra"))
            if(XmlNode *technique=extra->findNode("technique"))
         {
            XmlNode *spec=technique->findNode("spec_level"); if(!spec)spec=technique->findNode("specularLevel");
            if(spec)
            {
               if(XmlNode *n=spec->findNode("float"))Import(spec_level, *n);
               if(XmlNode *texture=spec->findNode("texture"))if(XmlParam *t=texture->findParam("texture"))
               {
                  specular_map_image_id=t->value;
                  REPA(param)if(param[i].sid==t->value){specular_map_image_id=param[i].image_id; break;}
               }
            }
            if(XmlNode *bump=technique->findNode("bump"))
            {
               if(XmlNode *texture=bump->findNode("texture"))if(XmlParam *t=texture->findParam("texture"))
               {
                  bump_map_image_id=t->value;
                  REPA(param)if(param[i].sid==t->value){bump_map_image_id=param[i].image_id; break;}
               }
            }
         }
      }
   }

   if(XmlNode *extra=node.findNode("extra"))
      if(XmlNode *technique=extra->findNode("technique"))
         if(XmlNode *dbl=technique->findNode("double_sided"))Import(double_sided, *dbl);
}
void DAE::Source::import(XmlNode &node)
{
   if(XmlParam *id=node.findParam("id"))T.id=id->asText();

   if(XmlNode *float_array=node.findNode("float_array"))Import(T.float_array, *float_array);
   if(XmlNode * name_array=node.findNode( "name_array"))Import(T. name_array, * name_array);

   if(XmlNode *technique=node.findNode("technique_common"))
      if(XmlNode *accessor=technique->findNode("accessor"))
      if(XmlParam *stride=accessor->findParam("stride"))T.stride=Max(1, stride->asInt());
}
void DAE::Input::import(XmlNode &node, Memb<Source> &source, Mesh *mesh)
{
   if(XmlParam *semantic=node.findParam("semantic"))
   {
      if(semantic->value=="INPUT"          )T.semantic=INPUT          ;else
      if(semantic->value=="OUTPUT"         )T.semantic=OUTPUT         ;else
      if(semantic->value=="INTERPOLATION"  )T.semantic=INTERPOLATION  ;else
      if(semantic->value=="VERTEX"         )T.semantic=VERTEX         ;else
      if(semantic->value=="POSITION"       )T.semantic=POSITION       ;else
      if(semantic->value=="NORMAL"         )T.semantic=NORMAL         ;else
      if(semantic->value=="TEXCOORD"       )T.semantic=TEXCOORD       ;else
      if(semantic->value=="JOINT"          )T.semantic=JOINT          ;else
      if(semantic->value=="WEIGHT"         )T.semantic=WEIGHT         ;else
      if(semantic->value=="INV_BIND_MATRIX")T.semantic=INV_BIND_MATRIX;
   }
   if(XmlParam *src=node.findParam("source"))
   {
      Str s=SkipStart(src->value, '#');
      if(semantic==VERTEX && mesh)REPA(mesh->vertices)if(Equal(s, mesh->vertices[i].id)){T.vertices=&mesh->vertices[i]; break;}
      if(!T.source)T.source=FindSource(source, s);
   }
   if(XmlParam *offset=node.findParam("offset"))T.offset=offset->asInt();
   if(XmlParam *   set=node.findParam(   "set"))T.   set=   set->asInt();
}
void DAE::Vertices::import(XmlNode &node, Mesh &mesh)
{
   if(XmlParam *id=node.findParam("id"))T.id=id->asText();

   FREPA(node.nodes)if(node.nodes[i].name=="input")inputs.New().import(node.nodes[i], mesh.source, &mesh);
}
void DAE::Triangles::import(XmlNode &node, Mesh &mesh, DAE &dae)
{
   if(XmlParam *material=node.findParam("material"))material_symbol=material->asText();

   if(XmlNode *p=node.findNode("p"))Import(T.p, *p);
   FREPA(node.nodes)if(node.nodes[i].name=="input")inputs.New().import(node.nodes[i], mesh.source, &mesh);
}
void DAE::PolyList::import(XmlNode &node, Mesh &mesh, DAE &dae)
{
   if(XmlParam *material=node.findParam("material"))material_symbol=material->asText();

   if(XmlNode *p     =node.findNode("p"     ))Import(T.p     , *p     );
   if(XmlNode *vcount=node.findNode("vcount"))Import(T.vcount, *vcount);
   FREPA(node.nodes)if(node.nodes[i].name=="input")inputs.New().import(node.nodes[i], mesh.source, &mesh);
}
void DAE::Polygons::import(XmlNode &node, Mesh &mesh, DAE &dae)
{
   if(XmlParam *material=node.findParam("material"))material_symbol=material->asText();

   FREPA(node.nodes)
   {
      XmlNode &n=node.nodes[i];
      if(n.name=="input")inputs.New().import(n, mesh.source, &mesh);else
      if(n.name=="p"    )Import(p.New(), n);
   }
}
void DAE::Mesh::import(XmlNode &node, DAE &dae)
{
   FREPA(node.nodes)
   {
      XmlNode &n=node.nodes[i];
      if(n.name=="source"   )source   .New().import(n        );else
      if(n.name=="vertices" )vertices .New().import(n, T     );else
      if(n.name=="triangles")triangles.New().import(n, T, dae);else
      if(n.name=="polylist" )poly_list.New().import(n, T, dae);else
      if(n.name=="polygons" )polygons .New().import(n, T, dae);
   }
}
void DAE::Geometry::import(XmlNode &node, DAE &dae)
{
   if(XmlParam *id  =node.findParam("id"  ))T.id  =id  ->asText();
   if(XmlParam *name=node.findParam("name"))T.name=name->asText();

   FREPA(node.nodes)if(node.nodes[i].name=="mesh")mesh.New().import(node.nodes[i], dae);
}
void DAE::Joint::import(XmlNode &node, Memb<Source> &source)
{
   FREPA(node.nodes)if(node.nodes[i].name=="input")inputs.New().import(node.nodes[i], source);
}
void DAE::VertexWeights::import(XmlNode &node, Memb<Source> &source)
{
   if(XmlNode *v     =node.findNode("v"     ))Import(T.v     , *v     );
   if(XmlNode *vcount=node.findNode("vcount"))Import(T.vcount, *vcount);
   FREPA(node.nodes)if(node.nodes[i].name=="input")inputs.New().import(node.nodes[i], source);

   REPA(inputs)if(inputs[i].source)switch(inputs[i].semantic)
   {
      case JOINT : joint =&inputs[i]; break;
      case WEIGHT: weight=&inputs[i]; break;
   }
}
void DAE::Skin::import(XmlNode &node, DAE &dae)
{
   if(XmlParam *source=node.findParam("source"))geometry_name=SkipStart(source->asText(), '#');

   if(XmlNode *bind_shape_matrix=node.findNode("bind_shape_matrix"))Import(T.bind_shape_matrix, *bind_shape_matrix);

   FREPA(node.nodes)
   {
      XmlNode &n=node.nodes[i];
      if(n.name=="source"        )source .New().import(n        );else
      if(n.name=="joints"        )joint  .New().import(n, source);else
      if(n.name=="vertex_weights")vtx_wgt.New().import(n, source);
   }

   REPA(joint)
   {
      Joint &joint=T.joint[i];
      REPA(joint.inputs)if(joint.inputs[i].source)switch(joint.inputs[i].semantic)
      {
         case JOINT          : name  =joint.inputs[i].source; break;
         case INV_BIND_MATRIX: matrix=joint.inputs[i].source; break;
      }
   }
}
void DAE::Controller::import(XmlNode &node, DAE &dae)
{
   if(XmlParam *id=node.findParam("id"))T.id=id->asText();
   FREPA(node.nodes)if(node.nodes[i].name=="skin")skin.New().import(node.nodes[i], dae);
}
void DAE::Controller::linkSkin(DAE &dae)
{
   FREPA(skin)
   {
      Skin &skin=T.skin[i];
      if(skin.geometry=dae.findGeometry(skin.geometry_name))skin.geometry->skin=&skin;
   }
}
void DAE::InstanceGeometry::import(XmlNode &node)
{
   if(XmlParam *url=node.findParam("url"))geometry_id=SkipStart(url->asText(), '#');

   FREPA(node.nodes)
   {
      XmlNode &bm=node.nodes[i]; if(bm.name=="bind_material")FREPA(bm.nodes)
      {
         XmlNode &tc=bm.nodes[i]; if(tc.name=="technique_common")FREPA(tc.nodes)
         {
            XmlNode &im=tc.nodes[i]; if(im.name=="instance_material")
            {
               InstanceMaterial &mtrl=instance_material.New();
               if(XmlParam *symbol=im.findParam("symbol"))mtrl.material_symbol=          symbol->asText();
               if(XmlParam *target=im.findParam("target"))mtrl.material_id    =SkipStart(target->asText(), '#');
            }
         }
      }
   }
}
void DAE::InstanceController::import(XmlNode &node)
{
   if(XmlParam *url=node.findParam("url"))controller_id=SkipStart(url->asText(), '#');

   FREPA(node.nodes)
   {
      XmlNode &bm=node.nodes[i]; if(bm.name=="bind_material")FREPA(bm.nodes)
      {
         XmlNode &tc=bm.nodes[i]; if(tc.name=="technique_common")FREPA(tc.nodes)
         {
            XmlNode &im=tc.nodes[i]; if(im.name=="instance_material")
            {
               InstanceMaterial &mtrl=instance_material.New();
               if(XmlParam *symbol=im.findParam("symbol"))mtrl.material_symbol=          symbol->asText();
               if(XmlParam *target=im.findParam("target"))mtrl.material_id    =SkipStart(target->asText(), '#');
            }
         }
      }
   }
}
void DAE::Node::import(XmlNode &node, Node *parent)
{
   T.parent=parent;
   Memc<Matrix> transform;

   if(XmlParam *id  =node.findParam("id"  ))T.id  = id  ->asText();
   if(XmlParam *name=node.findParam("name"))T.name= name->asText();
   if(XmlParam *sid =node.findParam("sid" ))T.sid = sid ->asText();
   if(XmlParam *type=node.findParam("type"))T.bone=(type->asText()=="JOINT");

   FREPA(node.nodes)
   {
      XmlNode &n=node.nodes[i];
      if(n.name=="node"               )T.nodes.New().import(n, this);else
      if(n.name=="translate"          ){Vec    temp; Import(temp, n); transform.New().setPos   (temp);}else
      if(n.name=="rotate"             ){Vec4   temp; Import(temp, n); transform.New().setRotate(temp.xyz, DegToRad(temp.w));}else
      if(n.name=="scale"              ){Vec    temp; Import(temp, n); transform.New().setScale (temp);}else
      if(n.name=="matrix"             ){Matrix temp; Import(temp, n); transform.New()=          temp ;}else
      if(n.name=="instance_geometry"  )instance_geometry  .New().import(n);else
      if(n.name=="instance_controller")instance_controller.New().import(n);
   }

   REPA(transform)local_matrix*=transform[i]; // transformations need to be performed from the end
}
void DAE::VisualScene::import(XmlNode &node, DAE &dae)
{
   if(XmlParam *id  =node.findParam("id"  ))T.id  =id  ->asText();
   if(XmlParam *name=node.findParam("name"))T.name=name->asText();

   if(XmlNode *extra=node.findNode("extra"))
      FREPA(extra->nodes)if(extra->nodes[i].name=="technique")
   {
      XmlNode &technique=extra->nodes[i];
      if(XmlNode *  end_time=technique.findNode(  "end_time")){Flt time; Import(time   , *  end_time); MAX(dae.force_duration, time);}
      if(XmlNode *frame_rate=technique.findNode("frame_rate"))           Import(dae.fps, *frame_rate);
   }
   FREPA(node.nodes)if(node.nodes[i].name=="node")T.nodes.New().import(node.nodes[i], null);
}
/******************************************************************************/
void DAE::loadAsset(XmlNode &node)
{
   if(XmlNode *unit=node.findNode("unit"))
      if(XmlParam *meter=unit->findParam("meter"))scale=meter->asFlt();

   if(XmlNode *up_axis=node.findNode("up_axis"))
      if(up_axis->data.elms())
   {
      if(up_axis->data[0]=="X_UP")T.up_axis=0;else
      if(up_axis->data[0]=="Y_UP")T.up_axis=1;else
      if(up_axis->data[0]=="Z_UP")T.up_axis=2;
   }
}
void DAE::loadLibraryAnimations(XmlNode &node)
{
   FREPA(node.nodes)if(node.nodes[i].name=="animation")animations.New().import(node.nodes[i], T);
}
void DAE::loadLibraryImages(XmlNode &node)
{
   FREPA(node.nodes)if(node.nodes[i].name=="image")images.New().import(node.nodes[i]);
}
void DAE::loadLibraryMaterials(XmlNode &node)
{
   FREPA(node.nodes)if(node.nodes[i].name=="material")materials.New().import(node.nodes[i]);
}
void DAE::loadLibraryEffects(XmlNode &node)
{
   FREPA(node.nodes)if(node.nodes[i].name=="effect")effects.New().import(node.nodes[i], T);
}
void DAE::loadLibraryGeometries(XmlNode &node)
{
   FREPA(node.nodes)if(node.nodes[i].name=="geometry")geometries.New().import(node.nodes[i], T);
}
void DAE::loadLibraryControllers(XmlNode &node)
{
   FREPA(node.nodes)if(node.nodes[i].name=="controller")controllers.New().import(node.nodes[i], T);
}
void DAE::loadLibraryVisualScenes(XmlNode &node)
{
   FREPA(node.nodes)if(node.nodes[i].name=="visual_scene")visual_scenes.New().import(node.nodes[i], T);
}
/******************************************************************************/
void DAE::Node::setBoneNodesFromSkin(DAE &dae)
{
   FREPAO(nodes).setBoneNodesFromSkin(dae);

   if(!bone)FREPA(dae.controllers)
   {
      Controller &ctrl=dae.controllers[i];
      FREPA(ctrl.skin)
      {
         Skin &skin=ctrl.skin[i];
         if(skin.name)REPA(skin.name->name_array)if(sid==skin.name->getName(i)){bone=true; return;}
      }
   }
}
Bool DAE::Node::setBoneNodesFromAnim(DAE &dae)
{
   Bool child_bone_mesh=false; FREPA(nodes)child_bone_mesh|=nodes[i].setBoneNodesFromAnim(dae);
   Bool mesh=(instance_geometry.elms() || instance_controller.elms()); // check in case the node has mesh (this is needed in case mesh has no skinning and would not setup a bone yet)
   child_bone_mesh|=mesh; // include from self
   if(!bone && child_bone_mesh && hasAnim())bone=true; // is not a bone, children have bone or mesh
   return child_bone_mesh || bone;
}
void DAE::setBoneNodesFromSkin()
{
   FREPA(visual_scenes)
   {
      VisualScene &scene=visual_scenes[i];
      FREPAO(scene.nodes).setBoneNodesFromSkin(T);
   }
}
void DAE::setBoneNodesFromAnim()
{
   FREPA(visual_scenes)
   {
      VisualScene &scene=visual_scenes[i];
      FREPAO(scene.nodes).setBoneNodesFromAnim(T);
   }
}
/******************************************************************************/
// LINK
/******************************************************************************/
void DAE::Material::linkEffect(DAE &dae)
{
   if(fx.is())REPA(dae.effects)if(Equal(fx, dae.effects[i].id))
   {
      effect=&dae.effects[i];
      break;
   }
}
void DAE::Channel::linkNode(DAE &dae)
{
   if(node=dae.findNode(target))node->channels.include(this);
}
Bool DAE::Channel::anyDifferent()C
{
   if(time)
   {
      if(transform){Matrix matrix=transform->getMatrix(0); REPA(time->float_array)if(!Equal(matrix, transform->getMatrix(i), EPS_ANIM_ANGLE, EPS_ANIM_POS))return true;}
      if(move     ){Vec    vec   =move     ->getVec   (0); REPA(time->float_array)if(!Equal(vec   , move     ->getVec   (i),                 EPS_ANIM_POS))return true;}
      if(move_x   ){Flt    f     =move_x   ->getFloat (0); REPA(time->float_array)if(!Equal(f     , move_x   ->getFloat (i),                 EPS_ANIM_POS))return true;}
      if(move_y   ){Flt    f     =move_y   ->getFloat (0); REPA(time->float_array)if(!Equal(f     , move_y   ->getFloat (i),                 EPS_ANIM_POS))return true;}
      if(move_z   ){Flt    f     =move_z   ->getFloat (0); REPA(time->float_array)if(!Equal(f     , move_z   ->getFloat (i),                 EPS_ANIM_POS))return true;}
      if( rot_x   ){Flt    f     = rot_x   ->getFloat (0); REPA(time->float_array)if(!Equal(f     ,  rot_x   ->getFloat (i), EPS_ANIM_ANGLE              ))return true;}
      if( rot_y   ){Flt    f     = rot_y   ->getFloat (0); REPA(time->float_array)if(!Equal(f     ,  rot_y   ->getFloat (i), EPS_ANIM_ANGLE              ))return true;}
      if( rot_z   ){Flt    f     = rot_z   ->getFloat (0); REPA(time->float_array)if(!Equal(f     ,  rot_z   ->getFloat (i), EPS_ANIM_ANGLE              ))return true;}
      if(scale    ){Flt    f     =scale    ->getFloat (0); REPA(time->float_array)if(!Equal(f     , scale    ->getFloat (i),        EPS_ANIM_SCALE       ))return true;}
   }
   return false;
}
void DAE::Animation::linkNode(DAE &dae)
{
   FREPAO(channel  ).linkNode(dae); // order is important
   FREPAO(animation).linkNode(dae); // order is important
}
void DAE::Node::linkBone(DAE &dae)
{
   if(bone)
   {
      bone_index=dae.bones.elms();
      dae.bones.add(this);
   }
   FREPAO(nodes).linkBone(dae); // order is important
}
void DAE::Node::linkSkin(DAE &dae)
{
   if(bone_index>=0 && sid.is())FREPA(dae.controllers)
   {
      Controller &ctrl=dae.controllers[i]; FREPA(ctrl.skin)
      {
         Skin &skin=ctrl.skin[i];
         if(skin.name)REPA(skin.name->name_array)if(sid==skin.name->getName(i))skin.bone_remap(i)=bone_index;
      }
   }
   REPAO(nodes).linkSkin(dae);
}
/******************************************************************************/
// ANIMATE
/******************************************************************************/
void DAE::Node::animate(Flt t)
{
   Bool    custom_orn=false;
   Matrix3 orn; orn.identity();

   color=1;
   anim_matrix=local_matrix; // node final position in local space
   REPA(channels) // order is important
   {
      Channel &channel=*channels[i]; if(channel.time)
      {
         Memc<Flt> &time=channel.time->float_array; if(time.elms())
         {
            Int prev=-1,
                next=-1;
            Flt step= 0;

            if(t<=time.first())prev=next=            0;else
            if(t>=time.last ())prev=next=time.elms()-1;else
            {
               Int l=0, r=time.elms(); for(; l<r; ){Int mid=UInt(l+r)/2; if(t<time[mid])r=mid;else l=mid+1;}
               prev=Mid(l-1, 0, time.elms()-1);
               next=Mid(l  , 0, time.elms()-1);
               step=LerpRS(time[prev], time[next], t);
            }

            if(channel.alpha       )          color.w    =Lerp(channel.alpha ->getFloat(prev), channel.alpha ->getFloat(next), step);
            if(channel.move        )    anim_matrix.pos  =Lerp(channel.move  ->getVec  (prev), channel.move  ->getVec  (next), step);
            if(channel.move_x      )    anim_matrix.pos.x=Lerp(channel.move_x->getFloat(prev), channel.move_x->getFloat(next), step);
            if(channel.move_y      )    anim_matrix.pos.y=Lerp(channel.move_y->getFloat(prev), channel.move_y->getFloat(next), step);
            if(channel.move_z      )    anim_matrix.pos.z=Lerp(channel.move_z->getFloat(prev), channel.move_z->getFloat(next), step);
            if(channel.rot_x       ){orn.rotateX(DegToRad(Lerp(channel.rot_x ->getFloat(prev), channel.rot_x ->getFloat(next), step))); custom_orn=true;}
            if(channel.rot_y       ){orn.rotateY(DegToRad(Lerp(channel.rot_y ->getFloat(prev), channel.rot_y ->getFloat(next), step))); custom_orn=true;}
            if(channel.rot_z       ){orn.rotateZ(DegToRad(Lerp(channel.rot_z ->getFloat(prev), channel.rot_z ->getFloat(next), step))); custom_orn=true;}
          //if(channel.scale_axis_r){}
            if(channel.scale       ){orn.scale  (         Lerp(channel.scale ->getFloat(prev), channel.scale ->getFloat(next), step) ); custom_orn=true;}
          //if(channel.scale_axis  ){}
            if(channel.transform   )
            {
               if(prev==next)
               {
                  anim_matrix=channel.transform->getMatrix(prev);
               }else
               {
                  Matrix prev_matrix=channel.transform->getMatrix(prev),
                         next_matrix=channel.transform->getMatrix(next);
               #if 1
                  anim_matrix.x  =Lerp(prev_matrix.x  , next_matrix.x  , step);
                  anim_matrix.y  =Lerp(prev_matrix.y  , next_matrix.y  , step);
                  anim_matrix.z  =Lerp(prev_matrix.z  , next_matrix.z  , step);
                  anim_matrix.pos=Lerp(prev_matrix.pos, next_matrix.pos, step);
               #else
                  // forces orthogonal matrix, seems like it's not required
                  OrientP o(Lerp(prev_matrix.pos, next_matrix.pos, step), Lerp(prev_matrix.z, next_matrix.z, step), Lerp(prev_matrix.y, next_matrix.y, step));
                  o.fix();
                  anim_matrix=o;
                  anim_matrix.x.setLength(Lerp(prev_matrix.x.length(), next_matrix.x.length(), step));
                  anim_matrix.y.setLength(Lerp(prev_matrix.y.length(), next_matrix.y.length(), step));
                  anim_matrix.z.setLength(Lerp(prev_matrix.z.length(), next_matrix.z.length(), step));
               #endif
               }
            }
         }
      }
   }
   if(custom_orn)anim_matrix.orn()=orn;
}
/******************************************************************************/
// CREATE MESH
/******************************************************************************/
void DAE::create(::Mesh &mesh, MemPtr<Int> part_material_index, ::Skeleton &skeleton, Geometry &geometry, Memb<InstanceMaterial> &im, Node &node)
{
   FREPA(geometry.mesh)
   {
      Mesh &geom_mesh=geometry.mesh[i];

      // set transform matrixes
      MemtN<Matrix, 256> matrixes;
      Int normal_matrixes=0, mirrored_matrixes=0; // matrixes can be sometimes "flipped" (x direction points in the wrong way that it should) in those cases faces are reversed, that's why I count them and later fix them
      if(Skin *skin=geometry.skin)
         if(skin->name && skin->matrix)
      {
         matrixes.setNum(skeleton.bones.elms()+VIRTUAL_ROOT_BONE);
         if(VIRTUAL_ROOT_BONE)matrixes[0].identity();

      #if CONVERT_BIND_POSE
         Matrix bind_shape_matrix_inv; skin->bind_shape_matrix.inverseNonOrthogonal(bind_shape_matrix_inv);
         REPA(skin->name->name_array)
         {
            Int bone_index=skin->bone_remap(i); if(InRange(bone_index, bones))
            {
               Node   &node=*bones   [bone_index];
               Matrix &m   = matrixes[bone_index+VIRTUAL_ROOT_BONE];
               skin->matrix->getMatrix(i).inverseNonOrthogonal(m); m*=bind_shape_matrix_inv; // m=GetTransform(skin.matrix->getMatrix(i), bind_shape_matrix_inv);
                                        m.inverseNonOrthogonal(m); m*=node.world_matrix    ; // m=GetTransform(                        m, node.world_matrix    );
               if(m.mirrored())mirrored_matrixes++;else normal_matrixes++;
            }
         }
      #else
         REPA(skin->name->name_array)
         {
            Int bone_index=skin->bone_remap(i); if(InRange(bone_index, bones))
            {
               Matrix &m=matrixes[bone_index+VIRTUAL_ROOT_BONE];
               m=skin->bind_shape_matrix; if(m.mirrored())mirrored_matrixes++;else normal_matrixes++;
            }
         }
      #endif
      }
      Bool reverse=(mirrored_matrixes>normal_matrixes);
      if(node.world_matrix.mirrored())reverse^=1; // required

      // set skinning
      Bool                    node_anim=false; // if node is animated
      Skinning                node_skin      ; // node     skinning
      Memc <Skinning>         skinning       ; // vertex   skinning
      MemtN<IndexWeight, 256> skin           ; // temporal weights

      if(geometry.skin && geometry.skin->vtx_wgt.elms())
      {
         VertexWeights &vtx_wgt=geometry.skin->vtx_wgt.first();
         if(vtx_wgt.joint && vtx_wgt.weight)
         {
            Memc<Byte> &bone_remap=geometry.skin->bone_remap;
            Int v_offset=0;
            skinning.setNum(vtx_wgt.vcount.elms()); // this is the number of vertexes
            FREPA(          vtx_wgt.vcount)
            {
               REPD(w, vtx_wgt.vcount[i]) // this is the number of weights for one vertex
               {
                  Int o=v_offset+vtx_wgt.joint ->offset; if(!InRange(o, vtx_wgt.v))break; Int  joint_index=vtx_wgt.v[o];
                      o=v_offset+vtx_wgt.weight->offset; if(!InRange(o, vtx_wgt.v))break; Int weight_index=vtx_wgt.v[o];
                  if(InRange(joint_index, bone_remap))skin.New().set(bone_remap[joint_index]+VIRTUAL_ROOT_BONE, vtx_wgt.weight->source->getFloat(weight_index));
                  v_offset+=vtx_wgt.inputs.elms();
               }
            #if 0 && DEBUG
               FREPA(skin)Log(S+skin[i].bone+"("+skin[i].weight+")  ");
               LogN();
            #endif
               SetSkin(skin, skinning[i].bone, skinning[i].blend, &skeleton); skin.clear();
            }
         }
      }else // doesn't have skinning but belongs to a bone
      {
         for(Node *cur=&node; cur; cur=cur->parent)if(cur->bone_index>=0) // find first node that's a bone
         {
            node_anim=true;
            node_skin.bone .set(cur->bone_index+VIRTUAL_ROOT_BONE, 0, 0, 0);
            node_skin.blend.set(                              255, 0, 0, 0); // 'blend.sum' must be equal to 255 !!
            break;
         }
      }

      // create mesh

      // triangles
      FREPA(geom_mesh.triangles)
      {
         Triangles &triangles=geom_mesh.triangles[i];
         Input     *position =null,
                   *normal   =null,
                   *tex[4]   ={null, null, null, null};
         FREPA(triangles.inputs)
         {
            Input &input=triangles.inputs[i];
            switch(input.semantic)
            {
               case POSITION: if(input.source && !position)position=&input; break;
               case NORMAL  : if(input.source && !normal  )normal  =&input; break;
               case TEXCOORD: if(input.source && InRange(input.set, tex))tex[input.set]=&input; break;
               case VERTEX  : if(Vertices *vertices=input.vertices)FREPA(vertices->inputs)
               {
                  Input &input=vertices->inputs[i];
                  switch(input.semantic)
                  {
                     case POSITION: if(input.source && !position)position=&input; break;
                     case NORMAL  : if(input.source && !normal  )normal  =&input; break;
                     case TEXCOORD: if(input.source && InRange(input.set, tex))tex[input.set]=&input; break;
                  }
               }break;
            }
         }
         FREPA(tex)if(i && tex[i] && !tex[i-1]){Swap(tex[i], tex[i-1]); i-=2;}

         if(triangles.p.elms() && triangles.inputs.elms() && position)
         {
            MeshPart &part=mesh.parts.New(); if(part_material_index)part_material_index.add(findMaterial(triangles.material_symbol, im));
            MeshBase &base=part.base;

            // set part
            Set(part.name, geometry.name);

            // set base
            Int tris=triangles.p.elms()/triangles.inputs.elms()/3;
            base.create(tris*3, 0, tris, 0, (normal?VTX_NRM:0)|(tex[0]?VTX_TEX0:0)|(tex[1]?VTX_TEX1:0)|(tex[2]?VTX_TEX2:0)|((skinning.elms()||node_anim)?VTX_SKIN:0));
            REPA(base.tri)
            {
               Int a=i*3, b=a+1, c=b+1,
                   ta=a*triangles.inputs.elms(),
                   tb=b*triangles.inputs.elms(),
                   tc=c*triangles.inputs.elms(),
                   p0=Safe(triangles.p, ta+position->offset),
                   p1=Safe(triangles.p, tb+position->offset),
                   p2=Safe(triangles.p, tc+position->offset);

               base.vtx.pos(a)=position->source->getVec(p0);
               base.vtx.pos(b)=position->source->getVec(p1);
               base.vtx.pos(c)=position->source->getVec(p2);

               if(skinning.elms())
               {
                  // out of range can happen on buggy exporters
                  if(InRange(p0, skinning)){base.vtx.matrix(a)=skinning[p0].bone; base.vtx.blend(a)=skinning[p0].blend;}else{base.vtx.matrix(a)=node_skin.bone; base.vtx.blend(a)=node_skin.blend;}
                  if(InRange(p1, skinning)){base.vtx.matrix(b)=skinning[p1].bone; base.vtx.blend(b)=skinning[p1].blend;}else{base.vtx.matrix(b)=node_skin.bone; base.vtx.blend(b)=node_skin.blend;}
                  if(InRange(p2, skinning)){base.vtx.matrix(c)=skinning[p2].bone; base.vtx.blend(c)=skinning[p2].blend;}else{base.vtx.matrix(c)=node_skin.bone; base.vtx.blend(c)=node_skin.blend;}
               }else
               if(node_anim)
               {
                  base.vtx.matrix(a)=node_skin.bone; base.vtx.blend(a)=node_skin.blend;
                  base.vtx.matrix(b)=node_skin.bone; base.vtx.blend(b)=node_skin.blend;
                  base.vtx.matrix(c)=node_skin.bone; base.vtx.blend(c)=node_skin.blend;
               }
               if(normal)
               {
                  base.vtx.nrm(a)=normal->source->getVec(Safe(triangles.p, ta+normal->offset));
                  base.vtx.nrm(b)=normal->source->getVec(Safe(triangles.p, tb+normal->offset));
                  base.vtx.nrm(c)=normal->source->getVec(Safe(triangles.p, tc+normal->offset));
               }
               if(tex[0])
               {
                  base.vtx.tex0(a)=tex[0]->source->getVec2(Safe(triangles.p, ta+tex[0]->offset));
                  base.vtx.tex0(b)=tex[0]->source->getVec2(Safe(triangles.p, tb+tex[0]->offset));
                  base.vtx.tex0(c)=tex[0]->source->getVec2(Safe(triangles.p, tc+tex[0]->offset));
               }
               if(tex[1])
               {
                  base.vtx.tex1(a)=tex[1]->source->getVec2(Safe(triangles.p, ta+tex[1]->offset));
                  base.vtx.tex1(b)=tex[1]->source->getVec2(Safe(triangles.p, tb+tex[1]->offset));
                  base.vtx.tex1(c)=tex[1]->source->getVec2(Safe(triangles.p, tc+tex[1]->offset));
               }
               if(tex[2])
               {
                  base.vtx.tex2(a)=tex[2]->source->getVec2(Safe(triangles.p, ta+tex[2]->offset));
                  base.vtx.tex2(b)=tex[2]->source->getVec2(Safe(triangles.p, tb+tex[2]->offset));
                  base.vtx.tex2(c)=tex[2]->source->getVec2(Safe(triangles.p, tc+tex[2]->offset));
               }
               base.tri.ind(i).set(a, b, c); if(reverse)base.tri.ind(i).reverse();
            }

            base.removeUnusedVtxs(); base.weldVtx(VTX_NRM|VTX_TEX_ALL|VTX_SKIN|VTX_COLOR, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down
            if( matrixes.elms())base.animate   (matrixes);else base.transform(node.world_matrix);
            if(!base.vtx.nrm ())base.setNormals();
         }
      }

      // poly_list
      FREPA(geom_mesh.poly_list)
      {
         PolyList &polys   =geom_mesh.poly_list[i];
         Input    *position=null,
                  *normal  =null,
                  *tex[4]  ={null, null, null, null};
         FREPA(polys.inputs)
         {
            Input &input=polys.inputs[i];
            switch(input.semantic)
            {
               case POSITION: if(input.source && !position)position=&input; break;
               case NORMAL  : if(input.source && !normal  )normal  =&input; break;
               case TEXCOORD: if(input.source && InRange(input.set, tex))tex[input.set]=&input; break;
               case VERTEX  : if(Vertices *vertices=input.vertices)FREPA(vertices->inputs)
               {
                  Input &input=vertices->inputs[i];
                  switch(input.semantic)
                  {
                     case POSITION: if(input.source && !position)position=&input; break;
                     case NORMAL  : if(input.source && !normal  )normal  =&input; break;
                     case TEXCOORD: if(input.source && InRange(input.set, tex))tex[input.set]=&input; break;
                  }
               }break;
            }
         }
         FREPA(tex)if(i && tex[i] && !tex[i-1]){Swap(tex[i], tex[i-1]); i-=2;}

         if(polys.vcount.elms() && polys.p.elms() && polys.inputs.elms() && position)
         {
            MeshPart &part=mesh.parts.New(); if(part_material_index)part_material_index.add(findMaterial(polys.material_symbol, im));
            MeshBase &base=part.base;

            // set part
            Set(part.name, geometry.name);

            // set base
            Int tris=0; REPA(polys.vcount)tris+=Max(0, polys.vcount[i]-2);
            base.create(tris*3, 0, tris, 0, (normal?VTX_NRM:0)|(tex[0]?VTX_TEX0:0)|(tex[1]?VTX_TEX1:0)|(tex[2]?VTX_TEX2:0)|((skinning.elms()||node_anim)?VTX_SKIN:0));

            Memb<Vtx> poly;

            Int v=0; tris=0; FREPA(polys.vcount)
            {
               Int vrts=polys.vcount[i]; FREP(vrts)
               {
                  Int  p  =Safe(polys.p, v+position->offset);
                  Vtx &vtx=poly.New();
                  if(skinning.elms())
                  {
                     if(InRange(p, skinning)) // out of range can happen on buggy exporters
                     {
                        vtx.matrix=skinning[p].bone ;
                        vtx.blend =skinning[p].blend;
                     }else
                     {
                        vtx.matrix=node_skin.bone ;
                        vtx.blend =node_skin.blend;
                     }
                  }else
                  if(node_anim)
                  {
                     vtx.matrix=node_skin.bone ;
                     vtx.blend =node_skin.blend;
                  }
                            vtx.pos   =position->source->getVec (p);
                  if(normal)vtx.nrm   =normal  ->source->getVec (Safe(polys.p, v+normal->offset));
                  if(tex[0])vtx.tex[0]=tex[0]  ->source->getVec2(Safe(polys.p, v+tex[0]->offset));
                  if(tex[1])vtx.tex[1]=tex[1]  ->source->getVec2(Safe(polys.p, v+tex[1]->offset));
                  if(tex[2])vtx.tex[2]=tex[2]  ->source->getVec2(Safe(polys.p, v+tex[2]->offset));
                  v+=polys.inputs.elms();
               }
               for(; poly.elms()>=3; )
               {
                  Int p0=tris*3+0,
                      p1=tris*3+1,
                      p2=tris*3+2;
                  Int o =0,
                      o0=(o  )            ,
                      o1=(o+1)%poly.elms(),
                      o2=(o+2)%poly.elms();
                  Vtx &v0=poly[o0],
                      &v1=poly[o1],
                      &v2=poly[o2];

                  base.vtx.pos(p0)=v0.pos;
                  base.vtx.pos(p1)=v1.pos;
                  base.vtx.pos(p2)=v2.pos;
                  
                  if(skinning.elms() || node_anim)
                  {
                     base.vtx.matrix(p0)=v0.matrix; base.vtx.blend(p0)=v0.blend;
                     base.vtx.matrix(p1)=v1.matrix; base.vtx.blend(p1)=v1.blend;
                     base.vtx.matrix(p2)=v2.matrix; base.vtx.blend(p2)=v2.blend;
                  }
                  if(normal)
                  {
                     base.vtx.nrm(p0)=v0.nrm;
                     base.vtx.nrm(p1)=v1.nrm;
                     base.vtx.nrm(p2)=v2.nrm;
                  }
                  if(tex[0])
                  {
                     base.vtx.tex0(p0)=v0.tex[0];
                     base.vtx.tex0(p1)=v1.tex[0];
                     base.vtx.tex0(p2)=v2.tex[0];
                  }
                  if(tex[1])
                  {
                     base.vtx.tex1(p0)=v0.tex[1];
                     base.vtx.tex1(p1)=v1.tex[1];
                     base.vtx.tex1(p2)=v2.tex[1];
                  }
                  if(tex[2])
                  {
                     base.vtx.tex2(p0)=v0.tex[2];
                     base.vtx.tex2(p1)=v1.tex[2];
                     base.vtx.tex2(p2)=v2.tex[2];
                  }

                  base.tri.ind(tris).set(p0, p1, p2); if(reverse)base.tri.ind(tris).reverse(); tris++;
                  poly.remove(o1, true);
               }
               poly.clear();
            }

            base.removeUnusedVtxs(); base.weldVtx(VTX_NRM|VTX_TEX_ALL|VTX_SKIN|VTX_COLOR, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down
            if( matrixes.elms())base.animate   (matrixes);else base.transform(node.world_matrix);
            if(!base.vtx.nrm ())base.setNormals();
         }
      }
      
      // polygons
      FREPA(geom_mesh.polygons)
      {
         Polygons &polys   =geom_mesh.polygons[i];
         Input    *position=null,
                  *normal  =null,
                  *tex[4]  ={null, null, null, null};
         FREPA(polys.inputs)
         {
            Input &input=polys.inputs[i]; switch(input.semantic)
            {
               case POSITION: if(input.source && !position)position=&input; break;
               case NORMAL  : if(input.source && !normal  )normal  =&input; break;
               case TEXCOORD: if(input.source && InRange(input.set, tex))tex[input.set]=&input; break;
               case VERTEX  : if(Vertices *vertices=input.vertices)FREPA(vertices->inputs)
               {
                  Input &input=vertices->inputs[i];
                  switch(input.semantic)
                  {
                     case POSITION: if(input.source && !position)position=&input; break;
                     case NORMAL  : if(input.source && !normal  )normal  =&input; break;
                     case TEXCOORD: if(input.source && InRange(input.set, tex))tex[input.set]=&input; break;
                  }
               }break;
            }
         }
         FREPA(tex)if(i && tex[i] && !tex[i-1]){Swap(tex[i], tex[i-1]); i-=2;}

         if(polys.p.elms() && polys.inputs.elms() && position)
         {
            MeshPart &part=mesh.parts.New(); if(part_material_index)part_material_index.add(findMaterial(polys.material_symbol, im));
            MeshBase &base=part.base;

            // set part
            Set(part.name, geometry.name);

            // set base
            Int tris=0; REPA(polys.p)tris+=Max(0, polys.p[i].elms()/polys.inputs.elms()-2);
            base.create(tris*3, 0, tris, 0, (normal?VTX_NRM:0)|(tex[0]?VTX_TEX0:0)|(tex[1]?VTX_TEX1:0)|(tex[2]?VTX_TEX2:0)|((skinning.elms()||node_anim)?VTX_SKIN:0));

            Memb<Vtx> poly;

            tris=0; FREPA(polys.p)
            {
               Memc<Int> &polys_p=polys.p[i];
               Int  vrts =polys_p.elms()/polys.inputs.elms(), v=0;
               FREP(vrts)
               {
                  Int  p  =Safe(polys_p, v+position->offset);
                  Vtx &vtx=poly.New();
                  if(skinning.elms())
                  {
                     if(InRange(p, skinning)) // out of range can happen on buggy exporters
                     {
                        vtx.matrix=skinning[p].bone ;
                        vtx.blend =skinning[p].blend;
                     }else
                     {
                        vtx.matrix=node_skin.bone ;
                        vtx.blend =node_skin.blend;
                     }
                  }else
                  if(node_anim)
                  {
                     vtx.matrix=node_skin.bone ;
                     vtx.blend =node_skin.blend;
                  }
                            vtx.pos   =position->source->getVec (p);
                  if(normal)vtx.nrm   =normal  ->source->getVec (Safe(polys_p, v+normal->offset));
                  if(tex[0])vtx.tex[0]=tex[0]  ->source->getVec2(Safe(polys_p, v+tex[0]->offset));
                  if(tex[1])vtx.tex[1]=tex[1]  ->source->getVec2(Safe(polys_p, v+tex[1]->offset));
                  if(tex[2])vtx.tex[2]=tex[2]  ->source->getVec2(Safe(polys_p, v+tex[2]->offset));
                  v+=polys.inputs.elms();
               }
               for(; poly.elms()>=3; )
               {
                  Int p0=tris*3+0,
                      p1=tris*3+1,
                      p2=tris*3+2;
                  Int o =0,
                      o0=(o  )            ,
                      o1=(o+1)%poly.elms(),
                      o2=(o+2)%poly.elms();
                  Vtx &v0=poly[o0],
                      &v1=poly[o1],
                      &v2=poly[o2];

                  base.vtx.pos(p0)=v0.pos;
                  base.vtx.pos(p1)=v1.pos;
                  base.vtx.pos(p2)=v2.pos;
                  
                  if(skinning.elms() || node_anim)
                  {
                     base.vtx.matrix(p0)=v0.matrix; base.vtx.blend(p0)=v0.blend;
                     base.vtx.matrix(p1)=v1.matrix; base.vtx.blend(p1)=v1.blend;
                     base.vtx.matrix(p2)=v2.matrix; base.vtx.blend(p2)=v2.blend;
                  }
                  if(normal)
                  {
                     base.vtx.nrm(p0)=v0.nrm;
                     base.vtx.nrm(p1)=v1.nrm;
                     base.vtx.nrm(p2)=v2.nrm;
                  }
                  if(tex[0])
                  {
                     base.vtx.tex0(p0)=v0.tex[0];
                     base.vtx.tex0(p1)=v1.tex[0];
                     base.vtx.tex0(p2)=v2.tex[0];
                  }
                  if(tex[1])
                  {
                     base.vtx.tex0(p0)=v0.tex[1];
                     base.vtx.tex0(p1)=v1.tex[1];
                     base.vtx.tex0(p2)=v2.tex[1];
                  }
                  if(tex[2])
                  {
                     base.vtx.tex0(p0)=v0.tex[2];
                     base.vtx.tex0(p1)=v1.tex[2];
                     base.vtx.tex0(p2)=v2.tex[2];
                  }

                  base.tri.ind(tris).set(p0, p1, p2); if(reverse)base.tri.ind(tris).reverse(); tris++;
                  poly.remove(o1, true);
               }
               poly.clear();
            }

            base.removeUnusedVtxs(); base.weldVtx(VTX_NRM|VTX_TEX_ALL|VTX_SKIN|VTX_COLOR, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down
            if( matrixes.elms())base.animate   (matrixes);else base.transform(node.world_matrix);
            if(!base.vtx.nrm ())base.setNormals();
         }
      }
   }
}
void DAE::Node::create(::Mesh &mesh, MemPtr<Int> part_material_index, ::Skeleton &skeleton, DAE &dae)
{
   FREPA(instance_geometry)
   {
      InstanceGeometry &ig=instance_geometry[i];
      if(Geometry *geometry=dae.findGeometry(ig.geometry_id))
         dae.create(mesh, part_material_index, skeleton, *geometry, ig.instance_material, T);
   }
   FREPA(instance_controller)
   {
      InstanceController &ic=instance_controller[i];
      if(Controller *controller=dae.findController(ic.controller_id))if(controller->skin.elms())if(Geometry *geometry=controller->skin[0].geometry)
         dae.create(mesh, part_material_index, skeleton, *geometry, ic.instance_material, T);
   }
   FREPA(nodes)nodes[i].create(mesh, part_material_index, skeleton, dae);
}
void DAE::create(::Mesh &mesh, MemPtr<Int> part_material_index, ::Skeleton &skeleton)
{
   FREPA(visual_scenes)FREPAD(j, visual_scenes[i].nodes)visual_scenes[i].nodes[j].create(mesh, part_material_index, skeleton, T);
   mesh.texScale(Vec2(1, -1), 0)
       .texScale(Vec2(1, -1), 1)
       .texScale(Vec2(1, -1), 2)
       .texScale(Vec2(1, -1), 3);
}
/******************************************************************************/
// CREATE SKELETON
/******************************************************************************/
void DAE::create(::Skeleton &skeleton, XAnimation *animation)
{
   skeleton.bones.setNum(bones.elms());
   FREPA(bones)
   {
      Node     &node=        *bones[i];
      SkelBone &sbon=skeleton.bones[i];
      Set(sbon.name,node.name);
          sbon.pos =node.world_matrix.pos;
          sbon.dir =node.world_matrix.x  ;
          sbon.perp=node.world_matrix.y  ;
          sbon.fix();

      Byte parent=0xFF; for(Node *cur=node.parent; cur; cur=cur->parent)if(cur->bone_index>=0){parent=cur->bone_index; break;} // find first parent which is a bone
      sbon.parent=parent;
   #if !CONVERT_BIND_POSE
      Int bone_index=i;
      FREPA(controllers)
      {
         Controller &controller=controllers[i];
         FREPA(controller.skin)
         {
            Skin &skin=controller.skin[i];
            if(skin.name && skin.matrix)
            {
               FREPA(skin.bone_remap)if(skin.bone_remap[i]==bone_index)
               {
                //Matrix bind_shape_matrix_inv; skin.bind_shape_matrix.inverseNonOrthogonal(bind_shape_matrix_inv);
                  Matrix m=skin.matrix->getMatrix(i); m.inverseNonOrthogonal(); 
                  sbon.pos = m.pos;
                  sbon.dir =!m.x  ;
                  sbon.perp=!m.y  ;
               }
            }
         }
      }
   #endif
   }

   // sort and remap
   MemtN<Byte, 256> old_to_new; skeleton.sortBones(old_to_new);
   Memc<Node*>    sorted_bones; sorted_bones.setNumZero(skeleton.bones.elms());
   FREPA(bones)
   {
      Node &node=*bones[i];
      node.adjustBoneIndex(InRange(node.bone_index, old_to_new) ? old_to_new[node.bone_index] : 0xFF);
      if(InRange(node.bone_index, sorted_bones))sorted_bones[node.bone_index]=&node;else node.bone_index=-1;
   }
   Swap(bones, sorted_bones);

   skeleton.setBoneLengths();
   if(animation)create(*animation, skeleton);
}
/******************************************************************************/
// CREATE ANIMATION
/******************************************************************************/
void DAE::create(XAnimation &animation, ::Skeleton &skeleton)
{
   Flt dt_avg=0;
   Int dt_num=0;
   FREPA(skeleton.bones)
   {
      Node           &node    =*bones(i);
      Memb<Channel*> &channels= node.channels;
      if(channels.elms())
      {
         SkelBone &sbon=skeleton .     bones[i];
         AnimBone &abon=animation.anim.bones.New(); abon.set(sbon.name);
         Matrix3   parent_matrix_inv; if(sbon.parent!=0xFF)skeleton.bones[sbon.parent].inverse(parent_matrix_inv);
         Matrix3      local_to_world; node.local_matrix.orn().inverseNonOrthogonal(local_to_world); local_to_world*=node.world_matrix.orn(); // GetTransform(node.local_matrix.orn(), node.world_matrix.orn());

         // gather keyframe time values
         Memc<Flt> times;
         REPA(channels)
         {
            Channel &channel=*channels[i];
            if(channel.time)FREPA(channel.time->float_array)times.include(channel.time->getFloat(i));
         }
         times.sort(Compare);

         Flt dt=0;
         abon.orns  .setNumZero(times.elms());
         abon.poss  .setNumZero(times.elms());
         abon.scales.setNumZero(times.elms());
      #if HAS_ANIM_COLOR
         abon.colors.setNumZero(times.elms());
      #endif
         FREPA(times)
         {
            Flt t=times[i]; // keyframe time position
            if(i)dt+=t-times[i-1];

            AnimKeys::Orn   &orn  =abon.orns  [i]; orn  .time=t;
            AnimKeys::Pos   &pos  =abon.poss  [i]; pos  .time=t;
            AnimKeys::Scale &scale=abon.scales[i]; scale.time=t;
         #if HAS_ANIM_COLOR
            AnimKeys::Color &color=abon.colors[i]; color.time=t;
         #endif

            node.animate(t);

            // orientation
            {
               Matrix3 anim;
               anim.x=node.anim_matrix.z;
               anim.y=node.anim_matrix.y;
               anim.z=node.anim_matrix.x;
               anim*=local_to_world;
               if(sbon.parent!=0xFF)anim*=parent_matrix_inv;
               orn.orn=anim;
               orn.orn.fix(); // orn.orn=Orient(!node.anim_matrix.x, !node.anim_matrix.y)*local_to_world; if(sbon.parent!=0xFF)orn.orn*=parent_matrix_inv; orn.orn.fix();
            }

            // position
            {
               pos.pos=node.anim_matrix.pos-node.local_matrix.pos;
               if(node.parent      )pos.pos*=node.parent->world_matrix.orn();
               if(sbon.parent!=0xFF)pos.pos*=parent_matrix_inv;
            }

            // scale
            {
               scale.scale.x=ScaleFactorR(node.anim_matrix.z.length()/node.local_matrix.z.length());
               scale.scale.y=ScaleFactorR(node.anim_matrix.y.length()/node.local_matrix.y.length());
               scale.scale.z=ScaleFactorR(node.anim_matrix.x.length()/node.local_matrix.x.length());
            }

         #if HAS_ANIM_COLOR
            // color
            {
               color.color=node.color;
            }
         #endif
         }
         if(times.elms()>2)dt/=times.elms()-1;
         if(dt){dt_avg+=dt; dt_num++;}
      }
   }
   if(dt_num)dt_avg/=dt_num;
   animation.fps=(fps ? fps : dt_avg ? 1/dt_avg : 0);
   animation.anim.length((force_duration>=0) ? force_duration : duration, false).setTangents().setRootMatrix();
}
/******************************************************************************/
// CREATE MATERIALS
/******************************************************************************/
void DAE::create(MemPtr<XMaterial> materials, Str path)
{
   FREPA(T.materials)
   {
       Material &src =T.materials[i];
      XMaterial &dest=  materials.New();
      if(src.effect)
      {
         Effect &fx=*src.effect;

         dest.name     = src.name;
         dest.color.xyz=  fx.color;
         dest.specular =((fx.spec_level>=0) ? fx.spec_level : fx.specular);
         dest.cull     = !fx.double_sided;

         if(Image *   color_map=findImage(fx.   color_map_image_id))dest.   color_map=   color_map->texture;
         if(Image *   alpha_map=findImage(fx.   alpha_map_image_id))dest.   alpha_map=   alpha_map->texture;
         if(Image *specular_map=findImage(fx.specular_map_image_id))dest.specular_map=specular_map->texture;
         if(Image *    bump_map=findImage(fx.    bump_map_image_id))dest.  normal_map=    bump_map->texture;

         dest.fixPath(path);
      }
   }
}
/******************************************************************************/
// MAIN
/******************************************************************************/
Bool ImportDAE(C Str &name, Mesh *mesh, Skeleton *skeleton, XAnimation *animation, MemPtr<XMaterial> materials, MemPtr<Int> part_material_index)
{
   if(mesh     )mesh     ->del();
   if(skeleton )skeleton ->del();
   if(animation)animation->del();
   materials          .clear();
   part_material_index.clear();

   XmlData xml; xml.load(name); if(XmlNode *COLLADA=xml.findNode("COLLADA")) // try even if 'load' fails (partial data may be available)
   {
      DAE dae;

      // import
      if(XmlParam *version=COLLADA->findParam("version"))dae.version=version->asInt();
      FREPA(COLLADA->nodes)
      {
         XmlNode &node=COLLADA->nodes[i];
         if(node.name=="asset"                )dae.loadAsset              (node);else
         if(node.name=="library_animations"   )dae.loadLibraryAnimations  (node);else
         if(node.name=="library_images"       )dae.loadLibraryImages      (node);else
         if(node.name=="library_materials"    )dae.loadLibraryMaterials   (node);else
         if(node.name=="library_effects"      )dae.loadLibraryEffects     (node);else
         if(node.name=="library_geometries"   )dae.loadLibraryGeometries  (node);else
         if(node.name=="library_controllers"  )dae.loadLibraryControllers (node);else
         if(node.name=="library_visual_scenes")dae.loadLibraryVisualScenes(node);
      }

      // create
      if(materials)
      {
         // link
         FREPA(dae.materials)dae.materials[i].linkEffect(dae);
         dae.create(materials, GetPath(name));
      }

      Skeleton temp, *skel=(skeleton ? skeleton : (mesh || animation) ? &temp : null); // if skel not specified, but we want mesh or animation, then we have to process it
      if(skel)
       {
         // link
         FREPA(dae.animations )dae.animations [i].linkNode(dae); // order is important, channels must be added in order (because there can be many channels for each node, for example rot,pos,..), call this before 'setBoneNodesFromAnim' because we're setting if node has animation or not
         FREPA(dae.controllers)dae.controllers[i].linkSkin(dae); // call before 'setBoneNodesFromAnim' because we're setting controller skin links
         dae.setBoneNodesFromSkin();
         dae.setBoneNodesFromAnim(); // call after 'linkNode' and 'linkSkin' and before 'linkBone'
         FREPA(dae.visual_scenes)FREPAD(j, dae.visual_scenes[i].nodes)dae.visual_scenes[i].nodes[j].linkBone   (dae); // order is important, bones must be added in order in which they are in the hierarchy
         FREPA(dae.visual_scenes)FREPAD(j, dae.visual_scenes[i].nodes)dae.visual_scenes[i].nodes[j].setWorldMatrix();
         FREPA(dae.visual_scenes)FREPAD(j, dae.visual_scenes[i].nodes)dae.visual_scenes[i].nodes[j].defaultPose   ();

         dae.create(*skel, animation);

         FREPA(dae.visual_scenes)FREPAD(j, dae.visual_scenes[i].nodes)dae.visual_scenes[i].nodes[j].linkSkin(dae); // 'linkSkin' after 'sortBones' inside "dae.create(*skel, .."

         if(mesh)dae.create(*mesh, part_material_index, *skel); // create mesh after 'linkSkin'
       //Animation *anim=(animation ? &animation->anim : null); RemoveNubBones(mesh, *skel, anim);

         // rescale
         if(dae.scale>=0.001f && dae.scale!=1)
         {
                         skel     ->     scale(dae.scale);
            if(animation)animation->anim.scale(dae.scale);
            if(mesh     )mesh     ->     scale(dae.scale);
         }

         // coordinate system
         if(dae.up_axis<0 || dae.up_axis==2) // unknown or Z
         {
                         skel     ->     rightToLeft();
            if(animation)animation->anim.rightToLeft(*skel);
            if(mesh     )mesh     ->     rightToLeft();
         }else
         if(dae.up_axis==1) // Y
         {
                         skel     ->     mirrorX();
            if(animation)animation->anim.mirrorX();
            if(mesh     )mesh     ->     mirrorX();
         }

         skel->setBoneTypes(); // must be called after transforms
         if(mesh     ){mesh     ->skeleton(skel).skeleton(null).setBox(); CleanMesh(*mesh);}
         if(animation) animation->anim.setBoneTypeIndexesFromSkeleton(*skel);
         if(skeleton ){skel     ->setBoneShapes(); if(skeleton!=skel)Swap(*skeleton, *skel);}
      }
      return true;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
