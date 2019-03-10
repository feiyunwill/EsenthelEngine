/******************************************************************************/
class Data // Custom Data class
{
   byte bytes[100];
   int  integer;
   flt  value;
}
/******************************************************************************/
Memc<Data> memc; // continuous based memory container
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   // adding elements
   {
      // Memc
      {
         Data &data=memc.New();
         data.integer=5;
      }
   }

   // iterating through all elements
   {
      // Memc
      for(int i=0; i<memc.elms(); i++)
      {
         Data &d=memc[i];
      }
      // Memc through better macro
      FREPA(memc)
      {
         Data &d=memc[i];
      }
   }

   // removing elements
   {
      // Memc
      {
         memc.remove(0, true); // remove 0th element, second parameter determines keeping order (you can read more about it in the header)
      }
   }

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
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(WHITE);
}
/******************************************************************************/
