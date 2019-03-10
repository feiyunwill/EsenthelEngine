/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Orient & Orient ::operator+=(C Orient  &orn) {dir+=orn.dir; perp+=orn.perp;               return T;}
OrientP& OrientP::operator+=(C OrientP &orn) {dir+=orn.dir; perp+=orn.perp; pos+=orn.pos; return T;}
/******************************************************************************/
Orient & Orient ::operator*=(Flt f) {dir*=f; perp*=f;         return T;}
OrientD& OrientD::operator*=(Dbl f) {dir*=f; perp*=f;         return T;}
OrientP& OrientP::operator*=(Flt f) {dir*=f; perp*=f; pos*=f; return T;}
OrientM& OrientM::operator*=(Flt f) {dir*=f; perp*=f; pos*=f; return T;}
/******************************************************************************/
Orient & Orient ::operator*=(C Vec  &v) {dir*=v; perp*=v; fix(); return T;}
OrientD& OrientD::operator*=(C VecD &v) {dir*=v; perp*=v; fix(); return T;}

OrientP& OrientP::operator*=(C Vec &v) {dir*=v; perp*=v; pos*=v; fix(); return T;}
OrientM& OrientM::operator*=(C Vec &v) {dir*=v; perp*=v; pos*=v; fix(); return T;}
/******************************************************************************/
Bool Orient ::operator==(C Orient  &orn)C {return dir==orn.dir && perp==orn.perp;}
Bool Orient ::operator!=(C Orient  &orn)C {return dir!=orn.dir || perp!=orn.perp;}
Bool OrientD::operator==(C OrientD &orn)C {return dir==orn.dir && perp==orn.perp;}
Bool OrientD::operator!=(C OrientD &orn)C {return dir!=orn.dir || perp!=orn.perp;}
Bool OrientP::operator==(C OrientP &orn)C {return dir==orn.dir && perp==orn.perp && pos==orn.pos;}
Bool OrientP::operator!=(C OrientP &orn)C {return dir!=orn.dir || perp!=orn.perp || pos!=orn.pos;}
Bool OrientM::operator==(C OrientM &orn)C {return dir==orn.dir && perp==orn.perp && pos==orn.pos;}
Bool OrientM::operator!=(C OrientM &orn)C {return dir!=orn.dir || perp!=orn.perp || pos!=orn.pos;}
/******************************************************************************/
Orient& Orient::mirrorX() {CHS(dir.x); CHS(perp.x); return T;}
Orient& Orient::mirrorY() {CHS(dir.y); CHS(perp.y); return T;}
Orient& Orient::mirrorZ() {CHS(dir.z); CHS(perp.z); return T;}

OrientD& OrientD::mirrorX() {CHS(dir.x); CHS(perp.x); return T;}
OrientD& OrientD::mirrorY() {CHS(dir.y); CHS(perp.y); return T;}
OrientD& OrientD::mirrorZ() {CHS(dir.z); CHS(perp.z); return T;}

OrientM& OrientM::mirrorX() {CHS(dir.x); CHS(perp.x); CHS(pos.x); return T;}
OrientM& OrientM::mirrorY() {CHS(dir.y); CHS(perp.y); CHS(pos.y); return T;}
OrientM& OrientM::mirrorZ() {CHS(dir.z); CHS(perp.z); CHS(pos.z); return T;}

OrientP& OrientP::mirrorX() {CHS(dir.x); CHS(perp.x); CHS(pos.x); return T;}
OrientP& OrientP::mirrorY() {CHS(dir.y); CHS(perp.y); CHS(pos.y); return T;}
OrientP& OrientP::mirrorZ() {CHS(dir.z); CHS(perp.z); CHS(pos.z); return T;}

Orient & Orient ::chs() {dir.chs(); perp.chs(); return T;}
OrientD& OrientD::chs() {dir.chs(); perp.chs(); return T;}

Orient & Orient ::rightToLeft() {dir.rightToLeft(); perp.rightToLeft();                    return T;}
OrientP& OrientP::rightToLeft() {dir.rightToLeft(); perp.rightToLeft(); pos.rightToLeft(); return T;}
/******************************************************************************/
Orient& Orient::mul(C Matrix3 &matrix, Bool normalized)
{
   dir *=matrix;
   perp*=matrix;
   if(!normalized){dir.normalize(); perp.normalize();}
   return T;
}
OrientD& OrientD::mul(C MatrixD3 &matrix, Bool normalized)
{
   dir *=matrix;
   perp*=matrix;
   if(!normalized){dir.normalize(); perp.normalize();}
   return T;
}
OrientP& OrientP::mul(C Matrix3 &matrix, Bool normalized)
{
   dir *=matrix;
   perp*=matrix;
   pos *=matrix;
   if(!normalized){dir.normalize(); perp.normalize();}
   return T;
}
OrientM& OrientM::mul(C Matrix3 &matrix, Bool normalized)
{
   dir *=matrix;
   perp*=matrix;
   pos *=matrix;
   if(!normalized){dir.normalize(); perp.normalize();}
   return T;
}
OrientP& OrientP::mul(C Matrix &matrix, Bool normalized)
{
   dir *=matrix.orn();
   perp*=matrix.orn();
   pos *=matrix;
   if(!normalized){dir.normalize(); perp.normalize();}
   return T;
}
OrientM& OrientM::mul(C Matrix &matrix, Bool normalized)
{
   dir *=matrix.orn();
   perp*=matrix.orn();
   pos *=matrix;
   if(!normalized){dir.normalize(); perp.normalize();}
   return T;
}
OrientP& OrientP::mul(C MatrixM &matrix, Bool normalized)
{
   dir *=matrix.orn();
   perp*=matrix.orn();
   pos *=matrix;
   if(!normalized){dir.normalize(); perp.normalize();}
   return T;
}
OrientM& OrientM::mul(C MatrixM &matrix, Bool normalized)
{
   dir *=matrix.orn();
   perp*=matrix.orn();
   pos *=matrix;
   if(!normalized){dir.normalize(); perp.normalize();}
   return T;
}
/******************************************************************************/
Orient& Orient::div(C Matrix3 &matrix, Bool normalized)
{
   dir .divNormalized(matrix);
   perp.divNormalized(matrix);
   if(!normalized){dir.normalize(); perp.normalize();}
   return T;
}
OrientD& OrientD::div(C MatrixD3 &matrix, Bool normalized)
{
   dir .divNormalized(matrix);
   perp.divNormalized(matrix);
   if(!normalized){dir.normalize(); perp.normalize();}
   return T;
}
OrientP& OrientP::div(C Matrix3 &matrix, Bool normalized)
{
   dir .divNormalized(matrix);
   perp.divNormalized(matrix);
   if(!normalized){dir.normalize(); perp.normalize(); pos.div(matrix);}else pos.divNormalized(matrix);
   return T;
}
OrientM& OrientM::div(C Matrix3 &matrix, Bool normalized)
{
   dir .divNormalized(matrix);
   perp.divNormalized(matrix);
   if(!normalized){dir.normalize(); perp.normalize(); pos.div(matrix);}else pos.divNormalized(matrix);
   return T;
}
OrientP& OrientP::div(C Matrix &matrix, Bool normalized)
{
   dir .divNormalized(matrix.orn());
   perp.divNormalized(matrix.orn());
   if(!normalized){dir.normalize(); perp.normalize(); pos.div(matrix);}else pos.divNormalized(matrix);
   return T;
}
OrientM& OrientM::div(C Matrix &matrix, Bool normalized)
{
   dir .divNormalized(matrix.orn());
   perp.divNormalized(matrix.orn());
   if(!normalized){dir.normalize(); perp.normalize(); pos.div(matrix);}else pos.divNormalized(matrix);
   return T;
}
OrientP& OrientP::div(C MatrixM &matrix, Bool normalized)
{
   dir .divNormalized(matrix.orn());
   perp.divNormalized(matrix.orn());
   if(!normalized){dir.normalize(); perp.normalize(); pos.div(matrix);}else pos.divNormalized(matrix);
   return T;
}
OrientM& OrientM::div(C MatrixM &matrix, Bool normalized)
{
   dir .divNormalized(matrix.orn());
   perp.divNormalized(matrix.orn());
   if(!normalized){dir.normalize(); perp.normalize(); pos.div(matrix);}else pos.divNormalized(matrix);
   return T;
}
/******************************************************************************/
Orient ::Orient (C OrientD &orn) {dir=orn.dir; perp=orn.perp;}
OrientD::OrientD(C Orient  &orn) {dir=orn.dir; perp=orn.perp; fix();} // call 'fix' to get high precision
OrientP::OrientP(C Orient  &orn) {dir=orn.dir; perp=orn.perp; pos.zero();}

Orient ::Orient (C Matrix3  &matrix) {dir=matrix.z; perp=matrix.y;}
Orient ::Orient (C MatrixD3 &matrix) {dir=matrix.z; perp=matrix.y;}
OrientD::OrientD(C Matrix3  &matrix) {dir=matrix.z; perp=matrix.y; fix();} // call 'fix' to get high precision
OrientD::OrientD(C MatrixD3 &matrix) {dir=matrix.z; perp=matrix.y;}
OrientP::OrientP(C Matrix3  &matrix) {dir=matrix.z; perp=matrix.y; pos.zero();}
OrientP::OrientP(C Matrix   &matrix) {dir=matrix.z; perp=matrix.y; pos=matrix.pos;}
OrientP::OrientP(C MatrixM  &matrix) {dir=matrix.z; perp=matrix.y; pos=matrix.pos;}
OrientM::OrientM(C Matrix3  &matrix) {dir=matrix.z; perp=matrix.y; pos.zero();}
OrientM::OrientM(C Matrix   &matrix) {dir=matrix.z; perp=matrix.y; pos=matrix.pos;}
OrientM::OrientM(C MatrixM  &matrix) {dir=matrix.z; perp=matrix.y; pos=matrix.pos;}

OrientP::OrientP(C Quaternion &q) : Orient(q), pos(VecZero) {}
OrientM::OrientM(C Quaternion &q) : Orient(q), pos(0      ) {}
/******************************************************************************/
Orient&  Orient ::zero() {dir.zero(); perp.zero();             return T;}
OrientD& OrientD::zero() {dir.zero(); perp.zero();             return T;}
OrientP& OrientP::zero() {dir.zero(); perp.zero(); pos.zero(); return T;}
OrientM& OrientM::zero() {dir.zero(); perp.zero(); pos.zero(); return T;}
/******************************************************************************/
Orient & Orient ::identity() {dir.set(0, 0, 1); perp.set(0, 1, 0);             return T;}
OrientD& OrientD::identity() {dir.set(0, 0, 1); perp.set(0, 1, 0);             return T;}
OrientP& OrientP::identity() {dir.set(0, 0, 1); perp.set(0, 1, 0); pos.zero(); return T;}
OrientM& OrientM::identity() {dir.set(0, 0, 1); perp.set(0, 1, 0); pos.zero(); return T;}
/******************************************************************************/
Orient & Orient ::setDir(C Vec  &dir) {T.dir=dir; T.perp=PerpN(dir); return T;}
OrientD& OrientD::setDir(C VecD &dir) {T.dir=dir; T.perp=PerpN(dir); return T;}

OrientP& OrientP::setPosDir(C Vec  &pos, C Vec &dir) {T.pos=pos; T.dir=dir; T.perp=PerpN(dir); return T;}
OrientM& OrientM::setPosDir(C VecD &pos, C Vec &dir) {T.pos=pos; T.dir=dir; T.perp=PerpN(dir); return T;}
/******************************************************************************/
Orient& Orient::fixPerp()
{
      perp=PointOnPlane(perp, dir);
   if(perp.normalize()<=EPS)perp=PerpN(dir); // if point on plane distance was very small, then it's very likely that due to precision issues, the vector will not be perpendicular to 'dir', so use epsilon comparison. This is very important, because we expect 'perp' to always be perpendicular to 'dir'
   return T;
}
OrientD& OrientD::fixPerp()
{
      perp=PointOnPlane(perp, dir);
   if(perp.normalize()<=EPSD)perp=PerpN(dir); // if point on plane distance was very small, then it's very likely that due to precision issues, the vector will not be perpendicular to 'dir', so use epsilon comparison. This is very important, because we expect 'perp' to always be perpendicular to 'dir'
   return T;
}
Bool Orient ::fix() {if(dir.normalize()){fixPerp(); return true;} return false;}
Bool OrientD::fix() {if(dir.normalize()){fixPerp(); return true;} return false;}
/******************************************************************************/
Orient& Orient::rotateDir(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      perp=perp*cos - cross()*sin;
   }
   return T;
}
Orient& Orient::rotatePerp(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      dir=cross()*sin + dir*cos;
   }
   return T;
}
Orient& Orient::rotateCross(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);
      Vec _dir=T.dir;

      dir = dir*cos - perp*sin;
      perp=_dir*sin + perp*cos;
   }
   return T;
}
/******************************************************************************/
Orient& Orient::rotateX(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt y=dir.y, z=dir.z;
      dir.y=y*cos - z*sin;
      dir.z=y*sin + z*cos;

      y=perp.y; z=perp.z;
      perp.y=y*cos - z*sin;
      perp.z=y*sin + z*cos;
   }
   return T;
}
Orient& Orient::rotateY(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt x=dir.x, z=dir.z;
      dir.x=x*cos + z*sin;
      dir.z=z*cos - x*sin;

      x=perp.x; z=perp.z;
      perp.x=x*cos + z*sin;
      perp.z=z*cos - x*sin;
   }
   return T;
}
Orient& Orient::rotateZ(Flt angle)
{
   if(Any(angle))
   {
      Flt cos, sin; CosSin(cos, sin, angle);

      Flt x=dir.x, y=dir.y;
      dir.x=x*cos - y*sin;
      dir.y=y*cos + x*sin;

      x=perp.x; y=perp.y;
      perp.x=x*cos - y*sin;
      perp.y=y*cos + x*sin;
   }
   return T;
}
/******************************************************************************/
Orient& Orient::rotateToDir(C Vec &dir)
{
   T.perp*=Matrix3().setRotation(T.dir, dir);
   T.dir  =dir;
   return T;
}
Orient& Orient::rotateToDir(C Vec &dir, Flt blend)
{
   Matrix3 m; m.setRotation(T.dir, dir, blend);
   T.perp*=m;
   T.dir *=m;
   return T;
}
/******************************************************************************/
void Orient::inverse(MatrixD3 &dest)C {OrientD(T).inverse(dest);} // use 'OrientD' to get high precision
void Orient::inverse(Matrix3  &dest)C
{
   Vec cross=T.cross();

   dest.x.x=cross.x;
   dest.x.y=perp .x;
   dest.x.z=dir  .x;

   dest.y.x=cross.y;
   dest.y.y=perp .y;
   dest.y.z=dir  .y;

   dest.z.x=cross.z;
   dest.z.y=perp .z;
   dest.z.z=dir  .z;
}
void OrientD::inverse(MatrixD3 &dest)C
{
   VecD cross=T.cross();

   dest.x.x=cross.x;
   dest.x.y=perp .x;
   dest.x.z=dir  .x;

   dest.y.x=cross.y;
   dest.y.y=perp .y;
   dest.y.z=dir  .y;

   dest.z.x=cross.z;
   dest.z.y=perp .z;
   dest.z.z=dir  .z;
}
void Orient::inverse(Orient &dest)C
{
   Vec cross=T.cross();
   Flt perp_z=perp.z; // remember in case 'dest' is 'this'

   dest.perp.x=cross.y;
   dest.perp.y=perp .y;
   dest.perp.z=dir  .y;

   dest.dir.x=cross.z;
   dest.dir.y=perp_z ;
   dest.dir.z=dir  .z;
}
void OrientD::inverse(OrientD &dest)C
{
   VecD cross=T.cross();
   Dbl  perp_z=perp.z; // remember in case 'dest' is 'this'

   dest.perp.x=cross.y;
   dest.perp.y=perp .y;
   dest.perp.z=dir  .y;

   dest.dir.x=cross.z;
   dest.dir.y=perp_z ;
   dest.dir.z=dir  .z;
}
Orient& Orient::inverse()
{
   Vec cross=T.cross();
   perp.x=cross.y;
   dir .x=cross.z;
   Swap(perp.z, dir.y);
   return T;
}
OrientD& OrientD::inverse()
{
   VecD cross=T.cross();
   perp.x=cross.y;
   dir .x=cross.z;
   Swap(perp.z, dir.y);
   return T;
}
/******************************************************************************/
void OrientP::draw(C Color &color, Flt size)C
{
   Vec blade_end =pos+perp*size,
        hilt_end =pos+dir*(size*0.30f),
       blade_side=    dir*(size*0.08f),
        hilt_side=cross()*(size*0.10f);
   VI.color(color);
   VI.line (pos           , blade_end);
   VI.line (pos-blade_side, blade_end);
   VI.line (pos+blade_side, blade_end);
   VI.line (pos+ hilt_side,  hilt_end);
   VI.line (pos- hilt_side,  hilt_end);
   VI.end  ();
}
void OrientM::draw(C Color &color, Flt size)C
{
   VecD blade_end =pos+perp*size,
         hilt_end =pos+dir*(size*0.30f);
   Vec  blade_side=    dir*(size*0.08f),
         hilt_side=cross()*(size*0.10f);
   VI.color(color);
   VI.line (pos           , blade_end);
   VI.line (pos-blade_side, blade_end);
   VI.line (pos+blade_side, blade_end);
   VI.line (pos+ hilt_side,  hilt_end);
   VI.line (pos- hilt_side,  hilt_end);
   VI.end  ();
}
/******************************************************************************/
void Orient::save(MemPtr<TextNode> nodes)C
{
   nodes.New().set("Dir" , dir );
   nodes.New().set("Perp", perp);
}
void OrientD::save(MemPtr<TextNode> nodes)C
{
   nodes.New().set("Dir" , dir );
   nodes.New().set("Perp", perp);
}
void OrientP::save(MemPtr<TextNode> nodes)C
{
   super::save(nodes);
   nodes.New().set("Pos", pos);
}
void OrientM::save(MemPtr<TextNode> nodes)C
{
   super::save(nodes);
   nodes.New().set("Pos", pos);
}
/******************************************************************************/
// AXIS ROLL
/******************************************************************************/
AxisRoll& AxisRoll::from(C Orient &start, C Orient &result)
{
   Vec cross=Cross(start.dir, result.dir); if(Flt sin=cross.normalize())
   {
      Flt cos=Dot(start.dir, result.dir);

      // set axis
      axis=cross*ACosSin(cos, sin);

      // set roll
      Orient  temp  =start*Matrix3().setRotateCosSin(cross, cos, sin);
      Matrix3 matrix=temp;
      roll=AngleDelta(Angle(temp.perp, matrix), Angle(result.perp, matrix));
   }else
   {
      // set axis
      axis.zero();

      // set roll
    //Matrix3 matrix=start; roll=AngleDelta(Angle(start.perp, matrix), Angle(result.perp, matrix                     ));
    //                      roll=AngleDelta(                     PI_2, Angle(result.perp, start.cross(), start.perp  )); // we don't need entire matrix, "Angle(start.perp, matrix)" gets converted to PI_2 because 'matrix' is made from 'start'
                            roll=                                     -Angle(result.perp, start.perp   , start.cross()); // instead of calculating angle by XY, we use YX which will get rid of PI_2 offset, however we need to change sign
   }
   return T;
}
AxisRoll& AxisRoll::mul(C Matrix3 &matrix)
{
   Flt length=axis.length();
              axis*=matrix;
              axis.setLength(length);
   return T;
}
/******************************************************************************/
Orient Lerp(C Orient &a, C Orient &b, Flt step)
{
   Orient o;
   o.dir =a.dir *Matrix3().setRotation(a.dir , b.dir , step);
   o.perp=a.perp*Matrix3().setRotation(a.perp, b.perp, step);
   o.fix();
   return o;
}
/******************************************************************************/
Bool Equal(C Orient  &a, C Orient  &b, Flt eps             ) {return Equal(a.dir, b.dir, eps) && Equal(a.perp, b.perp, eps);}
Bool Equal(C OrientD &a, C OrientD &b, Dbl eps             ) {return Equal(a.dir, b.dir, eps) && Equal(a.perp, b.perp, eps);}
Bool Equal(C OrientP &a, C OrientP &b, Flt eps, Flt pos_eps) {return Equal(a.dir, b.dir, eps) && Equal(a.perp, b.perp, eps) && Equal(a.pos, b.pos, pos_eps);}
Bool Equal(C OrientM &a, C OrientM &b, Flt eps, Dbl pos_eps) {return Equal(a.dir, b.dir, eps) && Equal(a.perp, b.perp, eps) && Equal(a.pos, b.pos, pos_eps);}
/******************************************************************************/
Orient GetAnimOrient(C Orient &orn, C Orient *parent)
{
   Orient O=orn; if(parent)O.div(*parent, true); // O/=parent
   return O;
}
Orient GetAnimOrient(C Orient &bone, C Matrix3 &bone_matrix, C Orient *parent, C Matrix3 *parent_matrix)
{
   Orient O=bone; O.mul(bone_matrix, true);
   if(parent && parent_matrix)
   {
      Orient transformed_parent=*parent; transformed_parent.mul(*parent_matrix, true);
       O.div(transformed_parent, true);
   }
 /*if(!normalized)*/{O.dir.normalize(); O.perp.normalize();}
   return O;
}
Vec GetAnimPos(C OrientP &bone, C Vec &bone_pos, C Orient &bone_orn, C Orient *parent, C Matrix *parent_matrix)
{
/* Following code is from "void UpdateBoneMatrix(AnimatedSkeleton &anim_skel, Int i)"
   bone._matrix.pos =-sbon.pos;
   bone._matrix.pos*=bone._matrix.orn();
   bone._matrix.pos+=sbon.pos;
   bone._matrix.pos+=(parent ? bone.pos*parent_matrix : bone.pos);
   bone._matrix    *=parent_transform_matrix;
   bone._matrix     =actor.matrix(); */

   Vec P=bone_pos;
   if(parent && parent_matrix)
   {
      P/=*parent_matrix;
      Matrix3 parent_=*parent, bone_matrix;
      Orient bone_orn_=bone_orn; bone_orn_.mul(parent_, true);
      bone.inverse(bone_matrix); bone_matrix.mul(Matrix3(bone_orn_));
      Vec p=-bone.pos; p*=bone_matrix; p+=bone.pos; P-=p;
      P/=parent_;
   }else
   {
      Matrix3 bone_matrix;
      bone.inverse(bone_matrix); bone_matrix.mul(Matrix3(bone_orn));
      Vec p=-bone.pos; p*=bone_matrix; p+=bone.pos; P-=p;
   }
   return P;
}
/******************************************************************************/
}
/******************************************************************************/
