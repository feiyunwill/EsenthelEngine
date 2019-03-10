/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace BVH{ // so local structures are unique
/******************************************************************************/
enum CHANNEL : Byte
{
   POS_X,
   POS_Y,
   POS_Z,
   ROT_X,
   ROT_Y,
   ROT_Z,
};
/******************************************************************************/
struct JOINT
{
   Bool          has_end;
   Byte          sbon_index;
   Int           abon_index;
   Str           name;
   Vec           offset, end_offset, pos;
   Memc<CHANNEL> channels;
   Memx<JOINT  > joints; // use 'Memx' because joints are referencing each other using 'parent'
   JOINT        *parent;

   JOINT() {has_end=false; sbon_index=0xFF; abon_index=-1; offset.zero(); parent=null;}

   void import(FileTextEx &f, JOINT *parent)
   {
      T.parent=parent;
      f.getLine(name);

      if(f.getIn())for(; f.level(); )
      {
         if(f.cur("OFFSET"))
         {
            f.get(offset);
            pos=(parent ? parent->pos+offset : offset);
         }else
         if(f.cur("CHANNELS"))
         {
            REP(f.getInt())
            {
               CChar   *channel=f.getWord();
               if(Equal(channel, "Xposition"))channels.New()=POS_X;else
               if(Equal(channel, "Yposition"))channels.New()=POS_Y;else
               if(Equal(channel, "Zposition"))channels.New()=POS_Z;else
               if(Equal(channel, "Xrotation"))channels.New()=ROT_X;else
               if(Equal(channel, "Yrotation"))channels.New()=ROT_Y;else
               if(Equal(channel, "Zrotation"))channels.New()=ROT_Z;
            }
         }else
         if(f.cur("JOINT"))joints.New().import(f, this);else
         if(f.cur("End"  ) && f.getIn())
         {
            for(; f.level(); )
            {
               if(f.cur("OFFSET"))
               {
                  f.get(end_offset);
                  has_end=true;
               }
            }
         }
      }
   }
   
   void toBone(Skeleton &skeleton, Byte parent_index)
   {
          sbon_index=skeleton.bones.elms();
      SkelBone &sbon=skeleton.bones.New ();
      Set(sbon.name  , name      );
          sbon.parent=parent_index;
          sbon.pos   =pos;

      Bool adjust=false;
      Vec  end;
      if(joints.elms())
      {
         end.zero(); REPA(joints)end+=joints[i].pos; end/=joints.elms();
         adjust=true;
      }else
      if(has_end)
      {
         end=pos+end_offset;
         adjust=true;
      }

      if(adjust)
      {
         Matrix3 m; m.setRotation(sbon.dir, !(end-pos));
         sbon.dir  *=m;
         sbon.perp *=m;
         sbon.length=Max(0.02f, Dist(end, sbon.pos));
      }

      FREPAO(joints).toBone(skeleton, sbon_index);
   }

   void toAnim(Animation &animation, Int frames)
   {
      if(channels.elms())
      {
         Bool has_pos=false,
              has_rot=false;
         REPA(channels)switch(channels[i])
         {
            case POS_X:
            case POS_Y:
            case POS_Z: has_pos=true; break;

            case ROT_X:
            case ROT_Y:
            case ROT_Z: has_rot=true; break;
         }
             abon_index=animation.bones.elms();
         AnimBone &abon=animation.bones.New(); abon.set((Str8)name);

         // clear anim keys data
         abon.poss.setNumZero(has_pos ? frames : 0);
         abon.orns.setNumZero(has_rot ? frames : 0);
      }
      FREPAO(joints).toAnim(animation, frames);
   }

   void loadFrame(Skeleton &skeleton, Animation &animation, FileTextEx &f, Int frame)
   {
      if(sbon_index!=0xFF && abon_index>=0)
      {
         SkelBone &sbon=skeleton .bones[sbon_index];
         AnimBone &abon=animation.bones[abon_index];
         Matrix3   m(1);

         FREPA(channels)switch(channels[i])
         {
            case POS_X: f.get(abon.poss[frame].pos.x); break;
            case POS_Y: f.get(abon.poss[frame].pos.y); break;
            case POS_Z: f.get(abon.poss[frame].pos.z); break;

            case ROT_X: m=Matrix().setRotateX(DegToRad(f.getFlt()))*m; break;
            case ROT_Y: m=Matrix().setRotateY(DegToRad(f.getFlt()))*m; break;
            case ROT_Z: m=Matrix().setRotateZ(DegToRad(f.getFlt()))*m; break;
         }

         if(abon.poss.elms())abon.poss[frame].time=frame;
         if(abon.orns.elms())abon.orns[frame].time=frame;

         if(abon.orns.elms())
         {
          C SkelBone *parent=skeleton.bones.addr(sbon.parent);
            Matrix3   parent_matrix_inv; if(parent)parent->inverse(parent_matrix_inv);

            Orient &orn=abon.orns[frame].orn;
            orn =sbon;
            orn*=m;
            if(parent)orn*=parent_matrix_inv;
         }
      }
      FREPAO(joints).loadFrame(skeleton, animation, f, frame);
   }
};
/******************************************************************************/
static void CreateAnimation(Skeleton &skeleton, XAnimation &animation, JOINT &root, FileTextEx &f)
{
   Int frames=0;
   Dbl frame_time=1;
   f.getWord();              f.get(frames    );
   f.getWord(); f.getWord(); f.get(frame_time);

   if(frames>0)
   {
                  root.toAnim   (animation.anim, frames);
      FREP(frames)root.loadFrame(skeleton, animation.anim, f, i);

      // finalize
      if(frame_time)animation.fps=1/frame_time;
      animation.anim.length(frames           , false); // set current length
      animation.anim.length(frames*frame_time, true ); // rescale keyframes
      animation.anim.setTangents().setRootMatrix();
   }
}
/******************************************************************************/
} // namespace BVH
/******************************************************************************/
Bool ImportBVH(C Str &name, Skeleton *skeleton, XAnimation *animation)
{
   if(skeleton )skeleton ->del();
   if(animation)animation->del();

   using namespace BVH;

   FileTextEx f; if(f.read(name))
   {
      Skeleton temp, *skel=(skeleton ? skeleton : animation ? &temp : null); // if skel not specified, but we want animation, then we have to process it
      JOINT    root;
      for(; f.level(); )
      {
         if(f.cur("ROOT"))
         {
            if(skel)
            {
               root.import(f, null);
               root.toBone(*skel, 0xFF);
            }
         }else
         if(f.cur("MOTION"))
         {
            if(animation)CreateAnimation(*skel, *animation, root, f);
            break;
         }
      }
      if(skel     ) skel     ->     mirrorX().sortBones().setBoneTypes();
      if(animation) animation->anim.mirrorX().setBoneTypeIndexesFromSkeleton(*skel);
      if(skeleton ){skel     ->setBoneShapes(); if(skeleton!=skel)Swap(*skeleton, *skel);}
      return true;
   }
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
