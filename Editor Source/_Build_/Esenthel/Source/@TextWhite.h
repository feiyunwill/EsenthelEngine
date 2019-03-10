/******************************************************************************/
/******************************************************************************/
class TextWhite : TextStyle // text style which resets its color upon changing skin with preference to black
{
   static ObjPtrs<TextWhite> texts;

   void       skinChanged();              
   TextWhite& reset      ();              
   TextWhite& operator=  (C TextStyle&ts);

   TextWhite();
  ~TextWhite();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
