/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
#define CC4_WAYPOINT CC4('W','P','N','T')
/******************************************************************************/
Cache<Waypoint> Waypoints("Waypoint");
/******************************************************************************/
// MANAGE
/******************************************************************************/
void Waypoint::zero()
{
   loop_mode=SINGLE;
}
Waypoint::Waypoint() {zero();}
Waypoint& Waypoint::del()
{
   points.del();
   zero();
   return T;
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Flt Waypoint::length()C
{
   if(points.elms()<=1)return 0;
   switch(loop_mode)
   {
      case LOOP     : return points.last().total_length + Dist(points.first().pos, points.last().pos);
      case PING_PONG: return points.last().total_length*2;
      default       : return points.last().total_length  ; // SINGLE
   }
}
/******************************************************************************/
Vec Waypoint::pos(Flt x, Bool smooth)C
{
   switch(points.elms())
   {
      case 0: return 0;
      case 1: return points.first().pos;

      default:
      {
         switch(loop_mode)
         {
            case LOOP:
            {
               x=Frac(x, length());

               // binary search
               Int l=0, r=points.elms(); for(; l<r; ){Int mid=UInt(l+r)/2; if(x<points[mid].total_length)r=mid;else l=mid+1;}
               if( l!=points.elms())
               {
                C Point &from=points[l-1],
                        &to  =points[l  ];
                  Flt    dl  =to.total_length-from.total_length; if(!dl)return from.pos;
                  Flt    s   =(x-from.total_length)/dl;
                  if(!smooth)return Lerp (                                     from.pos, to.pos,                                      s);
                             return Lerp4(points[Mod(l-2, points.elms())].pos, from.pos, to.pos, points[Mod(l+1, points.elms())].pos, s);
               }
               else // on the last virtual edge (last->first)
               {
                C Point &from=points.last (),
                        &to  =points.first();
                  Flt    dl  =Dist(from.pos,to.pos); if(!dl)return from.pos;
                  Flt    s   =(x-from.total_length)/dl;
                  if(!smooth)return Lerp (                                                 from.pos, to.pos,                                    s);
                             return Lerp4(points[Mod(points.elms()-2, points.elms())].pos, from.pos, to.pos, points[Mod(1, points.elms())].pos, s);
               }
            }break;

            case PING_PONG:
            {
               Flt length=T.length();
               x=Frac(x, length); if(x>length*0.5f)x=length-x;

               // binary search
               Int  l=0, r=points.elms(); for(; l<r; ){Int mid=UInt(l+r)/2; if(x<points[mid].total_length)r=mid;else l=mid+1;}
             C Point &from=points[l-1],
                     &to  =points[l  ];
               Flt    dl  =to.total_length-from.total_length; if(!dl)return from.pos;
               Flt    s   =(x-from.total_length)/dl;
               if(!smooth)return Lerp (                                     from.pos, to.pos,                                      s);
                          return Lerp4(points[Mod(l-2, points.elms())].pos, from.pos, to.pos, points[Mod(l+1, points.elms())].pos, s);
            }break;

            default: // SINGLE
            {
               if(x<=       0)return points.first().pos;
               if(x>=length())return points.last ().pos;

               // binary search
               Int  l=0, r=points.elms(); for(; l<r; ){Int mid=UInt(l+r)/2; if(x<points[mid].total_length)r=mid;else l=mid+1;}
             C Point &from=points[l-1],
                     &to  =points[l  ];
               Flt    dl  =to.total_length-from.total_length; if(!dl)return from.pos;
               Flt    s   =(x-from.total_length)/dl;
               if(!smooth)return Lerp (                         from.pos, to.pos,                                        s);
                          return Lerp4(points[Max(l-2, 0)].pos, from.pos, to.pos, points[Min(l+1, points.elms()-1)].pos, s);
            }break;
         }
      }break;
   }
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
void Waypoint::New(C Vec &pos)
{
   points.New().pos=pos;
   updateTotalLengths();
}
void Waypoint::New(Int i, C Vec &pos)
{
   points.NewAt(i).pos=pos;
   updateTotalLengths();
}
Waypoint& Waypoint::remove(Int i)
{
   if(InRange(i, points))
   {
      points.remove(i, true);
      updateTotalLengths();
   }
   return T;
}
/******************************************************************************/
Waypoint& Waypoint::rol    () {points. rotateOrder(-1); return updateTotalLengths();}
Waypoint& Waypoint::ror    () {points. rotateOrder( 1); return updateTotalLengths();}
Waypoint& Waypoint::reverse() {points.reverseOrder(  ); return updateTotalLengths();}
/******************************************************************************/
Waypoint& Waypoint::updateTotalLengths()
{
   if(points.elms())
   {
      Flt length=points[0].total_length=0;
      for(Int i=1; i<points.elms(); i++)
      {
         length+=Dist(points[i-1].pos, points[i].pos);
         points[i].total_length=length;
      }
   }
   return T;
}
/******************************************************************************/
// DRAW
/******************************************************************************/
static void DrawPoint(C Vec &pos, Flt radius)
{
   #define N 8
   Vec2 prev(radius, 0);
   REP(N)
   {
      Vec2 next; CosSin(next.x, next.y, i*(PI2/N)); next*=radius;
      VI.line(pos+prev. xy0(), pos+next. xy0());
      VI.line(pos+prev._0yx(), pos+next._0yx());
      prev=next;
   }
}
void Waypoint::draw(C Color &point_color, C Color &edge_color, Flt point_radius, Int edge_steps)C
{
   if(edge_color.a)
   {
      const Flt step=1.0f/edge_steps;
      const Vec up(0, point_radius, 0);

      VI.color(edge_color);
      REP(points.elms()-1)
      {
       C Point &a=points[i  ],
               &b=points[i+1];
         if(edge_steps>1)
         {
            Vec cur=b.pos; const Flt s=(b.total_length-a.total_length)*step;
            REP(edge_steps)
            {
               Vec next=pos(a.total_length+i*s, true);
               VI.line(cur-up, next-up);
               VI.line(cur+up, next+up);
               cur=next;
            }
         }else
         {
            VI.line(a.pos-up, b.pos-up);
            VI.line(a.pos+up, b.pos+up);
         }
      }
      if(loop_mode==LOOP && points.elms()>=3)
      {
       C Point &a=points.last (),
               &b=points.first();
         if(edge_steps>1)
         {
            Vec cur=b.pos; const Flt s=(length()-a.total_length)*step;
            REP(edge_steps)
            {
               Vec next=pos(a.total_length+i*s, true);
               VI.line(cur-up, next-up);
               VI.line(cur+up, next+up);
               cur=next;
            }
         }else
         {
            VI.line(a.pos-up, b.pos-up);
            VI.line(a.pos+up, b.pos+up);
         }
      }
      VI.end();
   }
   if(point_color.a && point_radius>EPS)
   {
      VI.color(point_color); REPA(points)DrawPoint(points[i].pos, point_radius);
      VI.end  (           );
   }
}
void DrawWaypoint(C Vec &pos, C Color &color, Flt radius)
{
   if(color.a)
   {
      VI.color(color); DrawPoint(pos, radius);
      VI.end  (     );
   }
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Waypoint::saveData(File &f)C
{
   f.cmpUIntV(0); // version
   if(points._saveRaw(f))
   {
      f.putByte(loop_mode);
      return f.ok();
   }
   return false;
}
Bool Waypoint::loadData(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 0: if(points._loadRaw(f))
      {
         loop_mode=LOOP_MODE(f.getByte());
         if(f.ok())return true;
      }break;
   }
   del(); return false;
}
/******************************************************************************/
Bool Waypoint::save(File &f)C
{
   f.putUInt(CC4_WAYPOINT);
   return saveData(f);
}
Bool Waypoint::load(File &f)
{
   if(f.getUInt()==CC4_WAYPOINT)return loadData(f);
   del(); return false;
}
/******************************************************************************/
Bool Waypoint::save(C Str &name)C
{
   File f; if(f.writeTry(name)){if(save(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
Bool Waypoint::load(C Str &name)
{
   File f; if(f.readTry(name))return load(f);
   del(); return false;
}
/******************************************************************************/
}}
/******************************************************************************/
