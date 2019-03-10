/******************************************************************************/
/******************************************************************************/
class ObjPaintClass : PropWin
{
   class Object
   {
      UID       id;
      ObjectPtr obj;
      Elm      *elm;

public:
   Object();
   };
   class Mtrl : ImageSkin
   {
      UID         id;
      MaterialPtr mtrl;
      Button      remove;

      virtual void update(C GuiPC &gpc)override;

public:
   Mtrl();
   };

   static void Hide      (ObjPaintClass &op);
   static Str  ElmName   (C UID &elm_id);    
   static void ClearObjs (ObjPaintClass&op);
   static void RemoveObj (ptr   p   );      
   static void RemoveMtrl(Mtrl &mtrl);      
   static void GetMtrlA  (ptr);             
   static void GetMtrlD  (ptr);             
   static void Copy      (ObjPaintClass&op);
   static void Paste     (ObjPaintClass&op);

   flt          collision, random_scale, scale_mul, align_normal, flow;
   Text         objects_t, allowed_mtrls_t, disallowed_mtrls_t;
   Region       objects_r, allowed_mtrls_r, disallowed_mtrls_r;
   Button                  allowed_mtrls_g, disallowed_mtrls_g, objects_clear, copy, paste;
   Memc<Object> objects_data;
   List<Object> objects_list;
   Memc<const_mem_addr Button> objects_remove; // watch out for const_mem_addr
   Memx<Mtrl>   allowed_mtrls, disallowed_mtrls;
   TextBlack    ts;

   bool ready    ()C;
   bool available()C;
   void clearObjs();
   void clearProj();
   void removeObj(int i);
   void removeMtrl(Mtrl &mtrl);
   void toGuiObjs();
   void toGuiMtrls(Memx<Mtrl> &mtrls, Region&region);
   void  copyDo();                                 
   void pasteDo();                                 
   void save(TextData &data)C;
   void load(C TextData &data);
   ObjPaintClass& create();
   virtual void update(C GuiPC &gpc)override;
   void erasing(C UID &elm_id);
   bool     hasMtrl (Memx<Mtrl> &mtrls, C MaterialPtr &mtrl   )C;
   bool     hasMtrl (Memx<Mtrl> &mtrls, C UID         &mtrl_id)C;
   bool includeMtrls(C MemPtr<UID> &elms, Memx<Mtrl> &mtrls);
   bool     hasObj (C UID &obj_id)C;                       
   bool includeObjs(C MemPtr<UID> &elms);
   void drag(Memc<UID> &elms, GuiObj *focus_obj, C Vec2 &screen_pos);

public:
   ObjPaintClass();
};
/******************************************************************************/
/******************************************************************************/
extern ObjPaintClass ObjPaint;
/******************************************************************************/
