/******************************************************************************/
namespace Edit{
/******************************************************************************/
STRUCT(SaveChanges , ClosableWindow)
//{
   struct Elm
   {
      Str name, display;
      UID id;
      Ptr user;

      // set
      Elm& set  (C Str &name, C Str &display, C UID &id=UIDZero, Ptr user=null) {T.name=name; T.display=display; T.id=id; T.user=user; return T;}
      Elm& save (Bool (*func)(Elm &elm)=null                                  ) {T._save =func;                                        return T;}
      Elm& close(void (*func)(Elm &elm)=null                                  ) {T._close=func;                                        return T;}

      // operations
      Bool doSaveClose();
      void doClose    ();

      Elm() {id.zero(); user=null; _save=null; _close=null;}

   private:
      Bool (*_save )(Elm &elm);
      void (*_close)(Elm &elm);
   };

   Memc<Elm> elms;
   List<Elm> list;
   Region    region;
   Button    save, close, cancel;

   // statics
   static void Save  (SaveChanges &sc) {sc.doSave ();}
   static void Close (SaveChanges &sc) {sc.doClose();}
   static void Cancel(SaveChanges &sc) {sc.hide   ();}

   // manage
   void create();

   // set
            void set(C MemPtr<Elm> &elms, void (*after_save_close)(Bool all_saved, Ptr   user)=null, Ptr   user=null);
   T1(TYPE) void set(C MemPtr<Elm> &elms, void (*after_save_close)(Bool all_saved, TYPE *user)     , TYPE *user     ) {set(elms, (void(*)(Bool, Ptr))after_save_close,  user);}
   T1(TYPE) void set(C MemPtr<Elm> &elms, void (*after_save_close)(Bool all_saved, TYPE &user)     , TYPE &user     ) {set(elms, (void(*)(Bool, Ptr))after_save_close, &user);}

   // operations
   void clear  ();
   void doSave ();
   void doClose();

   virtual Rect    sizeLimit(            )C;
   virtual Window& rect     (C Rect &rect) ;

   SaveChanges() {_after_save_close=null; _user=null;}

private:
   void (*_after_save_close)(Bool all_saved, Ptr user);
   Ptr    _user;
};
/******************************************************************************/
} // namespace
/******************************************************************************/
