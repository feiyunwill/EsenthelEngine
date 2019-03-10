/******************************************************************************

   Use 'Grab' to grab and smoothly drag 'Actor' to any custom position.

/******************************************************************************/
struct Grab // Actor Grabber
{
   // manage
   Grab& del   (                                           ); // delete
   Grab& create(Actor &actor, C Vec &local_pos, Flt power=5); // create, start grabbing 'actor' at its 'local_pos' local position with 'power' power

   // get / set
   Bool   is          (          )C {return _actor!=null;} // if  grabbing something
   Actor* grabbedActor(          )C {return _actor      ;} // get grabbed actor
   Vec    pos         (          )C {return _grab.pos() ;} // get world position of grabber
   Grab&  pos         (C Vec &pos);                        // move to 'pos' world position

#if EE_PRIVATE
   void zero();
#endif
  ~Grab() {del();}
   Grab();

private:
   Bool  _immediate;
   Flt   _power;
   Vec   _local_pos;
   Actor _grab, *_actor;
   Joint _joint;

   NO_COPY_CONSTRUCTOR(Grab);
};
/******************************************************************************/
