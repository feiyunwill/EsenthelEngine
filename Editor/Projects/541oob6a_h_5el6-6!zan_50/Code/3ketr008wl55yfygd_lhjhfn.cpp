/******************************************************************************

   'Number' is a number able to store very big values with big precision (Int or Real)

/******************************************************************************/
Number n0, n1, n2, n3, n4;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   // assign 1.0
   n0=1.0;
   // do some big operations
   n0*=100000; n0.sqrt(); n0+=123.456;
   n0*=100000; n0.sqrt(); n0+=123.456;
   n0*=100000; n0.sqrt(); n0+=123.456;
   // revert operations
   n0-=123.456; n0.sqr(); n0/=100000;
   n0-=123.456; n0.sqr(); n0/=100000;
   n0-=123.456; n0.sqr(); n0/=100000;
   // 'n0' should be back to 1.0

   // calculate sqrt(sqrt(sqrt(1000000000000000000000000000000.)))
   n1="1000000000000000000000000000000.";
   n1.sqrt();
   n1.sqrt();
   n1.sqrt();

   // NOTE: Number can be internally an integer or real (check Number.real)
   // any operation between int and int results also in int (any other in real)
   // when calculating Number(10)/3 you'll get 3 instead of 3.333..
   // to convert Number to be int or real use toInt() and toReal() methods
   {
      n2=Number(10  )/3; // int /int = int  (3)
      n3=Number(10.0)/3; // real/int = real (3.333..)

      n4=10;       // int
      n4.toReal(); // convert to real
      n4/=3;       // real/int = real (3.333..)
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
   D.clear(TURQ);
   D.text (0, 0.6, S+"n0 = "+n0.asDbl());
   D.text (0, 0.4, S+"n1 = "+n1.asDbl());
   D.text (0, 0.2, S+"n2 = "+n2.asDbl());
   D.text (0, 0.1, S+"n3 = "+n3.asDbl());
   D.text (0, 0.0, S+"n4 = "+n4.asDbl());
}
/******************************************************************************/
