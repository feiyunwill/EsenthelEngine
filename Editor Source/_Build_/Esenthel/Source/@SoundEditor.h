/******************************************************************************/
/******************************************************************************/
class SoundEditor : PropWin
{
   UID       elm_id;
   Elm      *elm;
   bool      changed,
             loop,
             seek_set;
   flt       volume, seek_pos;
   Sound     sound;
   Property *import_as, *length, *channels, *freq, *kbps, *size, *codec, *vol;
   Slider    progress;
   Button    play, locate, apply_vol;

   static void Locate(SoundEditor &editor);
   static void Seek  (SoundEditor &editor);
   static void Play  (SoundEditor &editor);

   class ImportAs
   {
      cchar8     *name;
      SOUND_CODEC codec;
      int         bit_rate; // -1=original, 0=auto, >0=relative to 44.1KHz stereo
   };
   static ImportAs Import_as[]
;
   static int ImportAsElms;

   static void SetImportAs(SoundEditor &editor, C Str &t);
   static Str GetImportAs(C SoundEditor &editor);
   static void ApplyVol(SoundEditor &editor);

   void create();
   void update(bool set_frac=true);
   void setInfo();

   virtual void update(C GuiPC &gpc)override;
   virtual void draw(C GuiPC &gpc)override;
   virtual SoundEditor& hide()override;    

   void flush();
   void setChanged();
   void set(Elm *elm);
   void activate(Elm *elm);     
   void toggle  (Elm *elm);     
   void closeElm(C UID &elm_id);
   void elmChanged(C UID &elm_id);
   void erasing(C UID &elm_id); 

public:
   SoundEditor();
};
/******************************************************************************/
/******************************************************************************/
extern SoundEditor SoundEdit;
/******************************************************************************/
