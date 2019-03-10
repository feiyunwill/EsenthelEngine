/******************************************************************************/
T1(TYPE) struct Node : TYPE // Custom Data Node
{
   Memx<Node> children; // children

   Node& New       (   ) {return children.New();} // create new child
   void  operator++(Int) {       children.New();} // create new child
};
/******************************************************************************/
