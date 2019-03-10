/******************************************************************************/
/******************************************************************************/
// TODO: make unavailable on Mobile
/******************************************************************************/
class FontEditor : PropWin
{
   class Params : EditFont
   {
      Str  sample_text;
      bool no_scale;

public:
   Params();
   };
   class Change : Edit::_Undo::Change
   {
      EditFont data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   UID          elm_id;
   Elm         *elm;
   bool         changed;
   Params       params;
   Font         font;
   Button       undo, redo, locate;
   ViewportSkin viewport;
   SyncEvent    event;
   Thread       thread;
   SyncLock     lock;
   Edit::Undo<Change> undos;   void undoVis();

  ~FontEditor(); // delete thread before other members

   static void Preview(Viewport &viewport);
          void preview();
   static bool Make(Thread &thread);
          bool make();

   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   static void ParamsFont      (Params &p, C Str &t);
   static void ParamsSize      (Params &p, C Str &t);
   static void ParamsScale     (Params &p, C Str &t);
   static void ParamsClearType (Params &p, C Str &t);
   static void ParamsSoftware  (Params &p, C Str &t);
   static void ParamsWeight    (Params &p, C Str &t);
   static void ParamsMinFilter (Params &p, C Str &t);
   static void ParamsDiagShadow(Params &p, C Str &t);
   static void ParamsMipMaps   (Params &p, C Str &t);
   static void ParamsShdBlur   (Params &p, C Str &t);
   static void ParamsShdOpacity(Params &p, C Str &t);
   static void ParamsShdSpread (Params &p, C Str &t);
   static void ParamsAscii     (Params &p, C Str &t);
   static void ParamsGerman    (Params &p, C Str &t);
   static void ParamsFrench    (Params &p, C Str &t);
   static void ParamsPolish    (Params &p, C Str &t);
   static void ParamsRussian   (Params &p, C Str &t);
   static void ParamsChinese   (Params &p, C Str &t);
   static void ParamsJapanese  (Params &p, C Str &t);
   static void ParamsKorean    (Params &p, C Str &t);
   static void ParamsCustomChar(Params &p, C Str &t);
   static void ParamsSampleText(Params &p, C Str &t);

   static void Undo  (FontEditor &editor);
   static void Redo  (FontEditor &editor);
   static void Locate(FontEditor &editor);

   void create();
   void toGui();  
   void setInfo();

   void stopThread();
   void refresh   ();

   virtual FontEditor& del ()override;
   virtual FontEditor& hide()override;

   void flush();
   void setChanged();
   void set(Elm *elm);
   void activate(Elm *elm);        
   void toggle  (Elm *elm);        
   void elmChanged(C UID &font_id);
   void erasing(C UID &elm_id);  

public:
   FontEditor();
};
/******************************************************************************/
/******************************************************************************/
extern FontEditor FontEdit;
/******************************************************************************/
