/******************************************************************************

   TextData is an alternative to XML, it will create smaller files.

/******************************************************************************/
int Param1;
Str Param2;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
void Save()
{
   TextData data;
   TextNode &node=data.getNode("Node"); // create new node
   node.nodes.New().set("Param1", "123");
   node.nodes.New().set("Param2", "Some text");
   data.save("data.txt"); // save to file
}
/******************************************************************************/
void Load()
{
   TextData data;
   if(data.load("data.txt")) // load from file
      if(TextNode *node=data.findNode("Node"))
   {
      if(TextNode *p=node.findNode("Param1"))Param1=p.asInt ();
      if(TextNode *p=node.findNode("Param2"))Param2=p.asText();
   }
}
/******************************************************************************/
bool Init()
{
   Save(); // first save sample data to   xml file
   Load(); // now   load this   data from xml file
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
   D.text (0, 0, S+Param1+" "+Param2); // display data obtained from file
}
/******************************************************************************/
