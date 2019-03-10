/******************************************************************************/
flt floats[8];

class Data
{
   byte bytes[100];
   flt  real;
   int  integer;
}
Data data[8];

Memc<Data> memc;
/******************************************************************************

   Custom comparing functions must be of "Int name(C TYPE &a, C TYPE &b)" format
   they receive references to 2 custom data elements
   they must return:
   -1 if 'a' should be before 'b'
   +1 if 'a' should be after  'b'
    0 if 'a' is the same as   'b'

/******************************************************************************/
int CompareData(C Data &a, C Data &b)
{
   if(a.real<b.real)return -1;
   if(a.real>b.real)return +1;
                    return  0;
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   REPA(floats)floats[i]     =RandomF(10); // fill 'floats'    with random values (0..10)
   REPA(data  )data  [i].real=RandomF(10); // fill 'data.real' with random values (0..10)
   REP (8     )memc  (i).real=RandomF(10); // fill 'memc.real' with random values (0..10)

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   if(Kb.bp(KB_SPACE)) // sort when space pressed
   {
           Sort(floats, Elms(floats)             ); // sort 'floats'
           Sort(data  , Elms(data  ), CompareData); // sort custom data by giving pointer to data, number of elements and custom comparing function
      memc.sort(                      CompareData); // sort custom data by giving memory block                        and custom comparing function
   }

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   D.text (0, 0.7, "Press Space to sort");
   
   Str s="floats:  "; FREPA(floats){s+=floats[i]     ; s+="  ";} D.text(0,  0.2, s); // draw 'floats'    in one string
       s="data:  "  ; FREPA(data  ){s+=data  [i].real; s+="  ";} D.text(0,  0.0, s); // draw 'data.real' in one string
       s="memc:  "  ; FREPA(memc  ){s+=memc  [i].real; s+="  ";} D.text(0, -0.2, s); // draw 'memc.real' in one string
}
/******************************************************************************/
