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
   XmlData xml;
   XmlNode &node=xml.getNode("Node"); // create new node
   node.params.New().set("Param1", "123");
   node.params.New().set("Param2", "Some text");
   xml.save("xml.txt"); // save to file
}
/******************************************************************************/
void Load()
{
   XmlData xml;
   xml.load("xml.txt"); // load from file
   if(XmlNode *node=xml.findNode("Node"))
   {
      if(XmlParam *param=node.findParam("Param1"))Param1=param.asInt ();
      if(XmlParam *param=node.findParam("Param2"))Param2=param.asText();
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
