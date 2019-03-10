/******************************************************************************

   Threads are programs (functions) which run simultaneously.

/******************************************************************************/
// global integers
int x,
    y[2];

// threads
Thread thread_x,
       thread_y0,
       thread_y1;
/******************************************************************************

   Thread functions must be of "bool Name(Thread &thread)" format.
   They return true when want to continue thread processing, and false when want to stop them.
   As input parameter thread functions receive reference to the 'Thread' which calls them.

/******************************************************************************/
bool ThreadFunc(Thread &thread)
{
   x++;            // simple increase x value
   Time.wait(100); // wait 100 ms
   return true;    // continue processing
}

bool ThreadFuncWithUserData(Thread &thread)
{
   y[(uintptr)thread.user]++; // make use of thread 'user' user data, and increase wanted 'y' (it will be y[0] or y[1])
   Time.wait(100);            // wait 100 ms
   return true;               // continue processing
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   // create threads
   thread_x .create(ThreadFunc                    ); // thread_x  will use 'ThreadFunc'             as its function
   thread_y0.create(ThreadFuncWithUserData, ptr(0)); // thread_y0 will use 'ThreadFuncWithUserData' as its function, and it will use '0' as its user data
   thread_y1.create(ThreadFuncWithUserData, ptr(1)); // thread_y1 will use 'ThreadFuncWithUserData' as its function, and it will use '1' as its user data
   return true;
}
/******************************************************************************/
void Shut()
{
   // delete threads
   thread_x .del();
   thread_y0.del();
   thread_y1.del();
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

   // draw values
   D.text(0, 0.2, S+"x: "   +x   );
   D.text(0, 0.1, S+"y[0]: "+y[0]);
   D.text(0, 0.0, S+"y[1]: "+y[1]);
}
/******************************************************************************/
