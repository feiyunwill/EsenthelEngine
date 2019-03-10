/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   void Code::del()
   {
      current.del();
      base   .del();
   }
   bool Code::save(File &f)C
   {
      f.cmpUIntV(3);
      f<<current<<base;
      return f.ok();
   }
   bool Code::load(File &f)
   {
      switch(f.decUIntV()) // 0 is special case of invalid data
      {
         case 3:
         {
            f>>current>>base;
            if(f.ok())return true;
         }break;

         case 2:
         {
            GetStr2(f, current);
            GetStr2(f, base   );
            if(f.ok())return true;
         }break;

         case 1:
         {
            GetStr(f, base   );
            GetStr(f, current);
            if(f.ok())return true;
         }break;
      }
      del(); return false;
   }
   bool Code::load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      del(); return false;
   }
/******************************************************************************/
