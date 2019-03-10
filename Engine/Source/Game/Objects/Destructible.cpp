/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
#define DEFAULT_MODE BREAKABLE // here you can specify the default mode for Destructible objects - STATIC or BREAKABLE
/******************************************************************************/
static void CreateJoint(Joint &joint, Actor &a, Actor *b)
{
   Flt mass=(b ? (a.mass()+b->mass()) : a.mass());
   joint.create(a, b).breakable(mass*3.1f, mass*0.22f);
}
static void CreateJoint(Joint &joint, Destructible &a, Destructible *b)
{
   if(a.actors.elms())
   {
      if(b) // joint attaches actor with other actor
      {
         if(b->actors.elms())CreateJoint(joint, a.actors.first(), &b->actors.first());
      }else // joint attaches actor with world
      {
         CreateJoint(joint, a.actors.first(), null);
      }
   }
}
/******************************************************************************/
Destructible::~Destructible()
{
}
Destructible::Destructible()
{
   mode          =STATIC;
   piece_index   =-1;
   mesh_variation= 0;
   scale         = 0;
   destruct_mesh =null;
}
/******************************************************************************/
// MANAGE
/******************************************************************************/
void Destructible::setUnsavedParams()
{
   if(base)if(C Param *destruct=base->findParam("destruct"))destruct_mesh=DestructMeshes.get(destruct->asID());
   mesh=null;
   if(mode==PIECE)
   {
      if(destruct_mesh && InRange(piece_index, destruct_mesh->parts()))mesh=&destruct_mesh->part(piece_index).mesh;
   }else
   {
      if(base)mesh=base->mesh();
   }
}
void Destructible::create(Object &obj)
{
   scale         =obj.scale();
   base          =obj.firstStored();
   mode          =DEFAULT_MODE;
   piece_index   =-1;
   mesh_variation=obj.meshVariationIndex();
   setUnsavedParams();

   Matrix matrix=obj.matrixFinal().normalize();

   switch(mode)
   {
      case BREAKABLE: setBreakable(matrix, AG_DEFAULT); break;
      case STATIC   : setStatic   (matrix, AG_DEFAULT); break;
      default       : Exit("Unrecognized Destructible Mode"); break;
   }
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Vec    Destructible::pos         (                ) {return actors.elms() ? actors.first().pos   (      )                 :              0;}
Matrix Destructible::matrix      (                ) {return actors.elms() ? actors.first().matrix(      )                 : MatrixIdentity;}
Matrix Destructible::matrixScaled(                ) {return actors.elms() ? actors.first().matrix(      ).scaleOrn(scale) : MatrixIdentity;}
void   Destructible::pos         (C Vec    &pos   ) {                 REPAO(actors)       .pos   (pos   );                                 }
void   Destructible::matrix      (C Matrix &matrix) {                 REPAO(actors)       .matrix(matrix);                                 }
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
void Destructible::setStatic(C Matrix &matrix, Byte actor_group)
{
   mode=STATIC;
   piece_index=-1;
   joints.del();
   actors.del();
   if(base && base->phys())actors.New().create(*base->phys(), 0, scale) // create main actor
                                       .matrix(matrix)
                                       .obj   (this)
                                       .group (actor_group);
}
void Destructible::setBreakable(C Matrix &matrix, Byte actor_group)
{
   mode=BREAKABLE;
   piece_index=-1;
   joints.del();
   actors.del();
   if(destruct_mesh) // create actors
   {
      FREP(destruct_mesh->parts())actors.New().create(destruct_mesh->part(i).phys, 1, scale)
                                              .matrix(matrix)
                                              .obj   (this)
                                              .group (actor_group);
      REPAO(actors).sleep(true); // put to sleep, call this after all actors have been created, in case creating other actors will wake up the neighbors
   }
}
struct ActorInfo
{
   Vec    vel, ang_vel;
   Matrix matrix;
   Byte   group;

   void set(C Matrix &matrix, C Vec &vel, C Vec &ang_vel, Byte group) {T.matrix=matrix; T.vel=vel; T.ang_vel=ang_vel; T.group=group;}
};
void Destructible::setPieces(Bool create_joints)
{
   if((mode==STATIC || mode==BREAKABLE) // not a piece
   && destruct_mesh                     // has information about destructible mesh
   && destruct_mesh->parts()            // destructible mesh has parts
   && type()>=0)                        // has information about OBJ_TYPE
   {
      Memc<Destructible*> pieces    ; // i-th element will point to i-th DestructMesh part
      Memc<ActorInfo    > actor_info; // i-th element will point to i-th DestructMesh part

      // create objects (pieces)
      Object op;
      op.type(true, ObjType.elmID(type()));
      FREP(destruct_mesh->parts()) // order is important
      {
         Destructible *destr=null;
         if(!i)destr=this;                                                        // 0-th    piece  is  created from this object
         else  destr=CAST(Destructible, World.objCreateNear(op, matrixScaled())); // rest of pieces are dynamically created from new objects
         pieces.add(destr);                                                       // add object to the lookup array
      }
      if(mode==BREAKABLE && actors.elms()==pieces.elms())FREPA(actors)actor_info.New().set(actors[i].matrix(), actors[i].vel(), actors[i].angVel(),                 actors[i].group()    ); // remember actor info from current object, to recreate later
      else                                                            actor_info.New().set(        T.matrix(),         VecZero,            VecZero, actors.elms() ? actors[0].group() : 0);

      // setup new objects
      joints.del(); // delete all joints of current object
      actors.del(); // delete all actors of current object
      UInt meshVariationID=(mesh ? mesh->variationID(mesh_variation) : 0);
      FREPA(pieces)if(Destructible *destr=pieces[i]) // create each piece
      {
         destr->mode                     = PIECE;
         destr->piece_index              = i;
         destr->base                     = base;
         destr->scale                    = scale;
         destr->destruct_mesh            = destruct_mesh;
         destr->mesh                     =&destruct_mesh->part(i).mesh;
         destr->mesh_variation           = destr->mesh->variationFind(meshVariationID);
         destr->actors.del().New().create( destruct_mesh->part(i).phys, 1, scale).obj(destr).sleepEnergy(0.35f); // set big sleeping energy threshold to more often put actors to sleep
      }

      // create joints
      if(create_joints)FREP(destruct_mesh->joints())
      {
         DestructMesh::Joint joint=destruct_mesh->joint(i);
         if(!InRange(joint.a, pieces) || !pieces[joint.a])Swap(joint.a, joint.b); // if 'a' is not valid then swap 'a' with 'b'
         if( InRange(joint.a, pieces) &&  pieces[joint.a])                        // if 'a' is     valid
         {
            Destructible *destr_a=pieces[joint.a];
            if(InRange(joint.b, pieces)) // b index is valid and points to other piece
            {
               if(Destructible *destr_b=pieces[joint.b])
               {
                  Joint &joint=destr_a->joints.New();
                              joint.destr=           destr_b ;
                  CreateJoint(joint.joint, *destr_a, destr_b);
               }
            }else // b index is not valid, so attach to world
            {
               Joint &joint=destr_a->joints.New();
               CreateJoint(joint.joint, *destr_a, null);
            }
         }
      }

      // adjust object actors
      FREPA(pieces)if(Destructible *destr=pieces[i])
      {
         ActorInfo &ai=actor_info[Min(i, actor_info.elms()-1)];
         destr->actors.first().matrix(ai.matrix).vel(ai.vel).angVel(ai.ang_vel).group(ai.group);
      }
   }
}
/******************************************************************************/
void Destructible::toStatic()
{
   if(mode==BREAKABLE)setStatic(matrix(), actors.elms() ? actors[0].group() : 0);
}
void Destructible::toBreakable()
{
   if(mode==STATIC)setBreakable(matrix(), actors.elms() ? actors[0].group() : 0);
}
void Destructible::toPieces()
{
   if(mode==STATIC || mode==BREAKABLE)setPieces(false);
}
/******************************************************************************/
// CALLBACKS
/******************************************************************************/
void Destructible::memoryAddressChanged()
{
   REPAO(actors).obj(this);
}
void Destructible::linkReferences()
{
   // create joints that attach actor with other objects (and their actors)
   REPA(joints)
   {
      Joint &joint=joints[i];
      Bool was_valid=   joint.destr.valid(); joint.destr.link(World);
      if( !was_valid && joint.destr.valid())CreateJoint(joint.joint, T, &joint.destr()); // create the joint only if reference was successfully linked at this moment (was not valid, but after linking it is valid)
   }
}
/******************************************************************************/
// UPDATE
/******************************************************************************/
#define EPS_ACTOR_ENERGY 0.0002f
Bool Destructible::update()
{
   switch(mode)
   {
      case BREAKABLE:
      {
         REPA(actors)if(!actors[i].sleep() && actors[i].energy()>EPS_ACTOR_ENERGY) // if at least one actor was moved
         {
            setPieces(true);
            break;
         }
      }break;

      case PIECE:
      {
         if(!destruct_mesh || !InRange(piece_index, destruct_mesh->parts()))return false;

         // pieces can reach very high velocities due to collisions, so clamp the velocity to maximum of 100
         Vec vel=actors.first().vel();
         if( vel.length()>100){vel.setLength(100); actors.first().vel(vel);}
      }break;
   }
   return true;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
UInt Destructible::drawPrepare()
{
   if(mesh)
   {
      Matrix matrix=matrixScaled(); if(Frustum(*mesh, matrix))
      {
         SetVariation(mesh_variation);
         mesh->draw(matrix, actors.elms() ? actors.first().vel() : VecZero, actors.elms() ? actors.first().angVel() : VecZero);
         SetVariation();
      }
   }
   return 0; // no additional render modes required
}
/******************************************************************************/
void Destructible::drawShadow()
{
   if(mesh)
   {
      Matrix matrix=matrixScaled(); if(Frustum(*mesh, matrix))
      {
         SetVariation(mesh_variation);
         mesh->drawShadow(matrix);
         SetVariation();
      }
   }
}
/******************************************************************************/
// ENABLE / DISABLE
/******************************************************************************/
void Destructible::disable()
{
   // freeze all actors
   if(mode!=STATIC) // static actor doesn't need to be freezed
   {
      REPAO(actors).kinematic(true); // freeze
   }
}
void Destructible::enable()
{
   // unfreeze all actors
   if(mode!=STATIC) // static actor doesn't need to be unfreezed
   {
                         REPAO(actors).kinematic(false); // unfreeze
      if(mode==BREAKABLE)REPAO(actors).sleep    (false); // put to sleep
   }
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Destructible::Joint::save(File &f)C {return destr.save(f);}
Bool Destructible::Joint::load(File &f)  {return destr.load(f);}
/******************************************************************************/
Bool Destructible::canBeSaved() {return super::canBeSaved() && actors.elms()>=1;} // save only if has at least 1 actor
/******************************************************************************/
Bool Destructible::save(File &f)
{
   DYNAMIC_ASSERT(actors.elms()>=1, "Destructible object doesn't have any actors");

   if(super::save(f))
   {
      f.cmpUIntV(0); // version

      f<<mode<<piece_index<<scale;
      f.putAsset(base.id());

      switch(mode)
      {
         case STATIC   :
         case BREAKABLE:
         {
            f<<matrix();
            f.putByte(actors.elms() ? actors.first().group() : 0);
         }break;

         case PIECE:
         {
            REPA(joints)if(joints[i].joint.broken())joints.remove(i); // remove unused joints
            if(!joints.save(f)             )return false;
            if(!actors.first().saveState(f))return false;
         }break;
      }
      f.putUInt(mesh ? mesh->variationID(mesh_variation) : 0);
      return f.ok();
   }
   return false;
}
/******************************************************************************/
Bool Destructible::load(File &f)
{
   if(super::load(f))switch(f.decUIntV()) // version
   {
      case 0:
      {
         f>>mode>>piece_index>>scale;
         base=f.getAssetID();
         setUnsavedParams();

         switch(mode)
         {
            case STATIC   :
            case BREAKABLE:
            {
               Matrix matrix; f>>matrix;
               Byte   group=f.getByte();
               switch(mode)
               {
                  case STATIC   : setStatic   (matrix, group); break;
                  case BREAKABLE: setBreakable(matrix, group); break;
               }
            }break;

            case PIECE:
            {
               if(!joints.load(f))return false;
               if(destruct_mesh && InRange(piece_index, destruct_mesh->parts()))
               {
                  Actor &actor=actors.New().create(destruct_mesh->part(piece_index).phys, 1, scale).obj(this);
                  if(!actor.loadState(f))return false;
               }else
               {
                  if(!Actor().loadState(f))return false;
               }

               // create joints that attach actor with world (joints which attach actor with other objects must be performed later in 'linkReferences')
               REPA(joints)
               {
                  Joint &joint=joints[i];
                  if(joint.destr.empty())CreateJoint(joint.joint, T, null); // if joint doesn't want to point to any other object, it means that it's a world joint
               }
            }break;

            default: return false;
         }

         UInt mesh_variation_id=f.getUInt(); mesh_variation=(mesh ? mesh->variationFind(mesh_variation_id) : 0);

         if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
