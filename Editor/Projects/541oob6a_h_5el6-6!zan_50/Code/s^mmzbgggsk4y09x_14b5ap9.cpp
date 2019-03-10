/******************************************************************************/
class Base // base class
{
   int value;

   virtual int func() {return 0;} // base codes
}
class Ext : Base // extended class
{
   virtual int func() {return 1;} // updated codes
}
/******************************************************************************/
class Abstract
{
   virtual int func()=NULL;
}

class NonAbstract0 : Abstract
{
   virtual int func() {return 0;}
}

class NonAbstract1 : Abstract
{
   virtual int func() {return 1;}
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   // replacing class
   {
      Memc<Base> memc;
      memc.replaceClass<Ext>(); // replace the class of elements stored in 'memc' (this however removes all previous elements from container)
      int value=memc.New().func(); // 'value' will be 1
          value=0;
   }
   
   // automatic casting
   {
      Memc<Ext >  ext; ext.New();
      Memc<Base> &base=ext; // automatic casting to container of base elements
      if(base.elms())
      {
         int value=base.first().func();
             value=0;
      }
   }

   // sorting
   {
      Memc<int> memc;
      memc.add(5);
      memc.add(3);
      memc.add(7);
      memc.sort(Compare); // sort by 'Compare' comparing function, 'memc' elements will now be sorted
      int value=memc[0];
          value=memc[1];
          value=memc[2];
          value=0;
   }

   // constant memory address for elements in all operations on 'Memx' memory container
   {
      Memx<Base> memx;
          REP(10)memx.New().value=0;      // create 10 elements with member 'value'=0
       Base &elm=memx.New(); elm.value=1; // create 1  element  with member 'value'=1 and remember elements memory address
          REP(10)memx.New().value=2;      // create 10 elements with member 'value'=2

      // remove all elements of "value!=1"
      {
         REPA(memx)if(memx[i].value!=1)memx.removeValid(i);
      }

      // after adding and removing elements, 'elm' memory address is still valid
      int value=elm.value;
          value=0;

      // methods for checking if element is present in container
      bool contains=memx.contains  (&elm);
                    memx.removeData(&elm);
           contains=memx.contains  (&elm);
           contains=false;
   }

   // storing elements of abstract classes
   {
      int value;
      MemcAbstract<Abstract> memc;
      memc.replaceClass<NonAbstract0>();
      value=memc.New().func();

      memc.replaceClass<NonAbstract1>();
      value=memc.New().func();

      value=0;
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
