/******************************************************************************/
struct Aggregate
{
#if EE_PRIVATE
   void create(Int    max_actors, Bool self_collision=true);
   void add   (Actor &actor);
#endif

   void         del();
  ~Aggregate() {del();}
   Aggregate() {_aggr=null;}

#if !EE_PRIVATE
private:
#endif
#if EE_PRIVATE
   PHYS_API(PxAggregate, void) *_aggr;
#else
   Ptr _aggr;
#endif

   NO_COPY_CONSTRUCTOR(Aggregate);
};
/******************************************************************************/
