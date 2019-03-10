/******************************************************************************/
class TextBlack : TextStyle // text style which resets its color upon changing skin with preference to black
{
   static ObjPtrs<TextBlack> texts;

   void       skinChanged();              
   TextBlack& reset      ();              
   TextBlack& operator=  (C TextStyle&ts);

   TextBlack();
  ~TextBlack();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
