/******************************************************************************/
/******************************************************************************/
class ObjClassEditor : ParamEditor
{
   Elm   *elm;
   UID    elm_id;
   Button locate;

   static void Locate(ObjClassEditor &editor);

   virtual void paramWindowHidden()override;

   void create();
   void flush();
   void set(Elm *elm);
   void activate(Elm *elm);       
   void toggle  (Elm *elm);       
   void elmChanged(C UID &elm_id);
   void erasing(C UID &elm_id); 

public:
   ObjClassEditor();
};
/******************************************************************************/
/******************************************************************************/
extern ObjClassEditor ObjClassEdit;
/******************************************************************************/
