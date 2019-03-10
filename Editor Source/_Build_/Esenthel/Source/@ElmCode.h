/******************************************************************************/
/******************************************************************************/
class ElmCode : ElmData
{
   uint undo(C ElmCode &src); // don't adjust 'ver' here because it also relies on the actual code data
   uint sync(C ElmCode &src); // don't adjust 'ver' here because it also relies on the actual code data

   void from(C Str &code);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
