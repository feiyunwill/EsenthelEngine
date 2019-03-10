/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
ObjParticles::~ObjParticles()
{
}
ObjParticles::ObjParticles()
{
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void ObjParticles::create(Object &obj)
{
   Particles *ps=null;
   if(Param *p=obj.findParam("particles")) // have Particles object specified
   {
      if(p->type==PARAM_STR)ps=ParticlesCache(p->asText());else
      if(p->type==PARAM_ID )ps=ParticlesCache(p->asID  ());
   }

   if(ps)particles.create(*ps);else // create from particles
   {  // otherwise check for all parameters manually
      if(Param *p=obj.findParam("Image"))
      {
         if(p->type==PARAM_STR)particles.image=p->asText();else
         if(p->type==PARAM_ID )particles.image=p->asID  ();
      }
      if(Param *p=obj.findParam("Palette Image"))
      {
         if(p->type==PARAM_STR)particles.palette_image=p->asText();else
         if(p->type==PARAM_ID )particles.palette_image=p->asID  ();
      }
      if(Param *p=obj.findParam("Image Frames X"    ))particles.image_x_frames  =Mid(p->asInt  (), 1, 65535);else particles.image_x_frames=1;
      if(Param *p=obj.findParam("Image Frames Y"    ))particles.image_y_frames  =Mid(p->asInt  (), 1, 65535);else particles.image_y_frames=1;
      if(Param *p=obj.findParam("Image Speed"       ))particles.image_speed     =    p->asFlt  ()           ;else particles.image_speed=1;
      if(Param *p=obj.findParam("Reborn"            ))particles.reborn          =    p->asBool ()           ;else particles.reborn=true;
      if(Param *p=obj.findParam("Emitter Life"      ))particles.emitter_life_max=Max(p->asFlt  (), 0       );else particles.emitter_life_max=0;
      if(Param *p=obj.findParam("Fade In Time"      ))particles.fade_in         =Max(p->asFlt  (), 0       );else particles.fade_in =1;
      if(Param *p=obj.findParam("Fade Out Time"     ))particles.fade_out        =Max(p->asFlt  (), 0       );else particles.fade_out=1;
      if(Param *p=obj.findParam("Palette Mode"      ))particles.palette         (    p->asBool ()          );else particles.palette(true);
      if(Param *p=obj.findParam("Palette Index"     ))particles.paletteIndex    (    p->asBool ()          );else particles.paletteIndex(false);
      if(Param *p=obj.findParam("Smooth Fade"       ))particles.smooth_fade     =    p->asBool ()           ;else particles.smooth_fade=false;
      if(Param *p=obj.findParam("Elements"          ))particles.p.setNumZero    (Max(p->asInt  (), 0      ));else particles.p.setNumZero(40);
      if(Param *p=obj.findParam("Color"             ))particles.color           =    p->asColor()           ;else particles.color.set(158, 0, 0, 0);
      if(Param *p=obj.findParam("Glow"              ))particles.glow            =Mid(p->asInt  (), 0, 255  );else particles.glow=0;
      if(Param *p=obj.findParam("Radius"            ))particles.radius          =    p->asFlt  ()           ;else particles.radius=0.048f;
      if(Param *p=obj.findParam("Radius Random"     ))particles.radius_random   =Max(p->asFlt  (), 0       );else particles.radius_random=0;
      if(Param *p=obj.findParam("Radius Growth"     ))particles.radius_growth   =Max(p->asFlt  (), 0       );else particles.radius_growth=1;
      if(Param *p=obj.findParam("Offset Range"      ))particles.offset_range    =    p->asFlt  ()           ;else particles.offset_range=0;
      if(Param *p=obj.findParam("Offset Speed"      ))particles.offset_speed    =    p->asFlt  ()           ;else particles.offset_speed=1;
      if(Param *p=obj.findParam("Life"              ))particles.life            =Max(p->asFlt  (), 0       );else particles.life=1.31f;
      if(Param *p=obj.findParam("Life Random"       ))particles.life_random     =Max(p->asFlt  (), 0       );else particles.life_random=0;
      if(Param *p=obj.findParam("Glue"              ))particles.glue            =Sat(p->asFlt  ()          );else particles.glue=0;
      if(Param *p=obj.findParam("Velocity Damping"  ))particles.damping         =Sat(p->asFlt  ()          );else particles.damping=0;
      if(Param *p=obj.findParam("Angular Velocity"  ))particles.ang_vel         =    p->asFlt  ()           ;else particles.ang_vel=0;
      if(Param *p=obj.findParam("Random Velocity"   ))particles.vel_random      =    p->asFlt  ()           ;else particles.vel_random=0;
      if(Param *p=obj.findParam("Initial Velocity X"))particles.vel_constant.x  =    p->asFlt  ()           ;else particles.vel_constant.x=0;
      if(Param *p=obj.findParam("Initial Velocity Y"))particles.vel_constant.y  =    p->asFlt  ()           ;else particles.vel_constant.y=0;
      if(Param *p=obj.findParam("Initial Velocity Z"))particles.vel_constant.z  =    p->asFlt  ()           ;else particles.vel_constant.z=0;
      if(Param *p=obj.findParam("Acceleration X"    ))particles.accel.x         =    p->asFlt  ()           ;else particles.accel.x=0;
      if(Param *p=obj.findParam("Acceleration Y"    ))particles.accel.y         =    p->asFlt  ()           ;else particles.accel.y=0;
      if(Param *p=obj.findParam("Acceleration Z"    ))particles.accel.z         =    p->asFlt  ()           ;else particles.accel.z=0;
      if(Param *p=obj.findParam("Inside Source"     ))particles.inside_shape    =    p->asBool ()           ;else particles.inside_shape=true;
      SHAPE_TYPE shape_type=SHAPE_NONE; if(Param *p=obj.findParam("Source Shape"))shape_type=p->asEnum(SHAPE_NONE);
      switch(shape_type)
      {
         default           : particles.source(Ball(0.5f)); break; // SHAPE_BALL
         case SHAPE_POINT  : particles.source(VecZero); break;
         case SHAPE_EDGE   : particles.source(Edge(Vec(-0.5f, 0, 0), Vec(0.5f, 0, 0))); break;
         case SHAPE_RECT   : particles.source(Rect(-0.5f, 0.5f)); break;
         case SHAPE_BOX    : particles.source(Box(0.5f)); break;
         case SHAPE_OBOX   : particles.source(OBox(Box(0.5f))); break;
         case SHAPE_CIRCLE : particles.source(Circle(0.5f)); break;
         case SHAPE_CAPSULE: particles.source(Capsule(0.3f, 1)); break;
         case SHAPE_TUBE   : particles.source(Tube(0.3f, 1)); break;
         case SHAPE_TORUS  : particles.source(Torus(0.3f, 0.1)); break;
         case SHAPE_CONE   : particles.source(Cone(0.5f, 0, 1, Vec(0, -0.5f, 0))); break;
         case SHAPE_PYRAMID: particles.source(Pyramid(0.5f, 1, Vec(0, 0.5f, 0), Vec(0, -1, 0))); break;
      }
   }
   Flt    scale =obj.scale();
   Matrix matrix=obj.matrixFinal();
   particles.matrix       =matrix;
   particles.radius      *=scale;
   particles.offset_range*=scale;
   particles.vel_random  *=scale;
   particles.vel_constant*=matrix.orn();
   particles.accel       *=matrix.orn();

   particles.resetFull();
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Vec    ObjParticles::pos   (                ) {return particles.matrix.pos       ;}
Matrix ObjParticles::matrix(                ) {return particles.matrix           ;}
void   ObjParticles::pos   (C Vec    &pos   ) {       particles.matrix.pos=pos   ;}
void   ObjParticles::matrix(C Matrix &matrix) {       particles.matrix    =matrix;}
/******************************************************************************/
// UPDATE
/******************************************************************************/
Bool ObjParticles::update()
{
   particles.update();
   return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
UInt ObjParticles::drawPrepare()
{
   return IndexToFlag(particles.renderMode()); // particles require additional render mode
}
void ObjParticles::drawBlend   () {particles.draw();}
void ObjParticles::drawPalette () {particles.draw();}
void ObjParticles::drawPalette1() {particles.draw();}
/******************************************************************************/
// IO
/******************************************************************************/
Bool ObjParticles::save(File &f)
{
   if(super::save(f))
   {
      f.cmpUIntV(0); // version

      if(particles.save(f, false))
         return f.ok();
   }
   return false;
}
Bool ObjParticles::load(File &f)
{
   if(super::load(f))switch(f.decUIntV()) // version
   {
      case 0:
      {
         if(particles.load(f))
            if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
