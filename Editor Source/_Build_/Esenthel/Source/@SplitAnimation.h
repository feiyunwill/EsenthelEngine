/******************************************************************************/
/******************************************************************************/
class SplitAnimation : ClosableWindow
{
   class Anim
   {
      Button   remove;
      CheckBox loop;
      TextLine name, from, to;
      
      static void Remove(Anim &anim);

      Anim();
      void pos(flt y);
   };
   UID        anim_id;
   Text       text, t_name, t_start, t_end, t_loop;
   Button     clipboard, split, clear, add_new;
   Region     region;
   Memx<Anim> anims;

   static void Clipboard(SplitAnimation &sa);
   static void Split    (SplitAnimation &sa);
   static void New      (SplitAnimation &sa);
   static void Clear    (SplitAnimation &sa);
   static void Hide     (SplitAnimation &sa);

   void splitDo();
   void clearDo();
   void addNew ();
   void setList();
   void create();
   void activate(C UID &elm_id);
   static bool Create(int &data, C Str&key, ptr user); // initial occurence is zero
   static bool IsNumber(C Str &str);                
   void add(C Str &text);
   void drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos);

public:
   SplitAnimation();
};
/******************************************************************************/
/******************************************************************************/
extern SplitAnimation SplitAnim;
/******************************************************************************/
