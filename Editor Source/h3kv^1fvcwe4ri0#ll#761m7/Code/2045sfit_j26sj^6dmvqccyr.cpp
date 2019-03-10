/******************************************************************************/
class Code // Source Code File
{
   Str current, base;

   void del()
   {
      current.del();
      base   .del();
   }

   bool save(File &f)C
   {
      f.cmpUIntV(3);
      f<<current<<base;
      return f.ok();
   }
   bool load(File &f)
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
   bool load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f);
      del(); return false;
   }
}
/******************************************************************************/
