/******************************************************************************/
/******************************************************************************/
class Version // value randomized each time the data is changed indicating that client/server sync is needed
{
   uint ver;

   bool operator==(C Version &v)C;
   bool operator!=(C Version &v)C;
   bool operator! (            )C;
        operator bool(         )C;

   void randomize(); // avoid zero and previous value

public:
   Version();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
