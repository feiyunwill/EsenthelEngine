/******************************************************************************/
/******************************************************************************/
class EraseRemovedElms : ClosableWindow
{
   static void OK(EraseRemovedElms &ere);

   class Elm
   {
      Str name;
      UID id;
      
      void create(C ::Elm &src);

public:
   Elm();
   };

   TextNoTest text;
   Button     ok, cancel;
   Region     region;
   Memc<Elm>  data;
   List<Elm>  list;

   void create();
   virtual EraseRemovedElms& show()override;
   virtual EraseRemovedElms& hide()override;
   virtual void update(C GuiPC &gpc)override;
};
/******************************************************************************/
/******************************************************************************/
extern EraseRemovedElms EraseRemoved;
/******************************************************************************/
