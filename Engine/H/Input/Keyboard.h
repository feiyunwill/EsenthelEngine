/******************************************************************************

   Use 'Kb' to access Keyboard input.

   Use 'KbSc' to describe keyboard shortcuts.

/******************************************************************************/
struct Keyboard // Keyboard Input
{
   struct Key
   {
      Char   c;
      KB_KEY k;
      Byte   flags;

      Bool any    ()C {return  c || k                ;} // if any character or key pressed
      Bool ctrl   ()C {return  FlagTest(flags, CTRL );} // if any  Control pressed
      Bool shift  ()C {return  FlagTest(flags, SHIFT);} // if any  Shift   pressed
      Bool alt    ()C {return  FlagTest(flags, ALT  );} // if any  Alt     pressed
      Bool win    ()C {return  FlagTest(flags, WIN  );} // if any  Win     pressed
      Bool lalt   ()C {return  FlagTest(flags, LALT );} // if Left Alt     pressed
      Bool first  ()C {return  FlagTest(flags, FIRST);} // if this is the first press of the key (if false then it's a repeated press)
      Bool ctrlCmd()C {return APPLE ? win () : ctrl();} // if any Ctrl is on (on platforms other than Apple), and if any Command is on (on Apple platforms)
      Bool winCtrl()C {return APPLE ? ctrl() : win ();} // if any Win  is on (on platforms other than Apple), and if any Control is on (on Apple platforms)

      Bool operator()(Char   c)C {return T.c==c;} // use simple == instead of 'EqualCS' for performance reasons
      Bool operator()(Char8  c)C {return T.c==c;} // use simple == instead of 'EqualCS' for performance reasons
      Bool operator()(KB_KEY k)C {return T.k==k;}
      Bool first     (KB_KEY k)C {return T.k==k && first();} // if this is the first press of 'k' key

      void clear() {c='\0'; k=KB_NONE; flags=0;}

      void eat()C; // eat this key input from this frame so it will not be processed by the remaining codes in frame

      enum
      {
         CTRL =1<<0,
         SHIFT=1<<1,
         ALT  =1<<2,
         WIN  =1<<3,
         LALT =1<<4,
         FIRST=1<<5,
      };
   };

   // get
#if EE_PRIVATE
   Key k;
#else
   const Key k; // key pressed in this frame
#endif
   Bool kf(KB_KEY k)C {return T.k.first(k);} // if key 'k' is pressed and it's the first press

   Bool b (KB_KEY k)C {return ButtonOn(_button[k]);} // if key 'k' is on
   Bool bp(KB_KEY k)C {return ButtonPd(_button[k]);} // if key 'k' pushed   in this frame
   Bool br(KB_KEY k)C {return ButtonRs(_button[k]);} // if key 'k' released in this frame
   Bool bd(KB_KEY k)C {return ButtonDb(_button[k]);} // if key 'k' double clicked
#if EE_PRIVATE
   void _assert() {ASSERT(1<<(8*SIZE(KB_KEY))==ELMS(_button));} // because we use 'KB_KEY' as index to '_button' in methods above
   Bool visibleWanted()C;
#endif

   Bool ctrl ()C {return _ctrl ;} // if any Ctrl  is on (this is equal to "b(KB_LCTRL ) || b(KB_RCTRL )")
   Bool shift()C {return _shift;} // if any Shift is on (this is equal to "b(KB_LSHIFT) || b(KB_RSHIFT)")
   Bool alt  ()C {return _alt  ;} // if any Alt   is on (this is equal to "b(KB_LALT  ) || b(KB_RALT  )")
   Bool win  ()C {return _win  ;} // if any Win   is on (this is equal to "b(KB_LWIN  ) || b(KB_RWIN  )")
#if EE_PRIVATE // this checks the most recent state, to be used while receiving inputs from the system (unlike methods above which use cached values at the start of the frame)
   Bool anyCtrl ()C {return ButtonOn(_button[KB_LCTRL ]|_button[KB_RCTRL ]);}
   Bool anyShift()C {return ButtonOn(_button[KB_LSHIFT]|_button[KB_RSHIFT]);}
   Bool anyAlt  ()C {return ButtonOn(_button[KB_LALT  ]|_button[KB_RALT  ]);}
   Bool anyWin  ()C {return ButtonOn(_button[KB_LWIN  ]|_button[KB_RWIN  ]);}
#endif

   Char   keyChar(KB_KEY key)C; // get key character, example: keyChar(KB_SPACE) -> ' '
  CChar8* keyName(KB_KEY key)C; // get key name     , example: keyName(KB_SPACE) -> "Space"

   Bool hwAvailable (          ); // if hardware keyboard is available
   Bool softCoverage(Rect &rect); // get soft keyboard (on-screen) coverage, false if no screen keyboard is currently displayed

   KB_KEY qwerty(KB_KEY qwerty)C; // convert key from QWERTY layout to layout of current keyboard

   // operations
   void eat(          ); // eat all    input from this frame so it will not be processed by the remaining codes in frame
   void eat(Char   c  ); // eat 'c'    input from this frame so it will not be processed by the remaining codes in frame
   void eat(Char8  c  ); // eat 'c'    input from this frame so it will not be processed by the remaining codes in frame
   void eat(KB_KEY key); // eat 'key'  input from this frame so it will not be processed by the remaining codes in frame
   void eatKey(       ); // eat 'Kb.k' input from this frame so it will not be processed by the remaining codes in frame

#if EE_PRIVATE
   void nextInQueue(); // proceed to next key from the buffer, same like 'nextKey' but without 'eatKey'
   Key* nextKeyPtr(); // get next key in the queue, without calling 'eatKey' and without moving it to 'Kb.k' but just return a pointer to it, null if none
#endif
   void nextKey(); // specify that you've processed 'Kb.k' and would like to proceed to the next key in the queue

   void queue(C Key &key); // manually add 'key' to the buffer to be processed later

   Bool exclusive()C {return _exclusive;}   void exclusive(Bool on); // get/set keyboard exclusive mode (which disables Windows key on Windows platform), default=false

   void requestTextInput(); // call this each frame if you wish to manually process the keyboard input in text format, this enables IMM on Windows and displays screen keyboard on Mobile platforms, this is automatically enabled if Gui keyboard focus is on 'TextLine' or 'TextBox' object
#if EE_PRIVATE
   void refreshTextInput();
   void     setTextInput(C Str &text, Int start, Int end, Bool password);
#endif

   // IMM (Windows Input Method Manager) control
#if EE_PRIVATE
   Bool       imm         ()C;   void imm      (Bool enable); // get/set if disable/enable the ability to use Windows Input Method Manager, this method is automatically called be the engine when keyboard focus is switched to text editing capable gui object
#endif
   Bool       immNative   ()C;   void immNative(Bool native); // get/set if native typing mode is currently enabled
   Int        immCursor   ()C {return _imm_cursor   ;}        // get     IMM cursor position
 C VecI2    & immSelection()C {return _imm_selection;}        // get     IMM clause selection range, where x=min index, y=max index
 C Str      & immBuffer   ()C {return _imm_buffer   ;}        // get     IMM text buffer
 C Memc<Str>& immCandidate()C {return _imm_candidate;}        // get     IMM candidate list

#if EE_PRIVATE
   // manage
   void init   ();
   void del    ();
   void create ();
   void acquire(Bool on);
   void setLayout();

   // operations
   void clear  ();
   void push   (KB_KEY key, Int scan_code);
   void queue  (Char   chr, Int scan_code);
   void release(KB_KEY key);
   void update ();

   void swappedCtrlCmd(Bool swapped);   Bool swappedCtrlCmd()C {return _swapped_ctrl_cmd;} // set/get if Ctrl is swapped with Cmd key under Mac OS, enable this method if you have swapped Ctrl with Cmd key under System Preferences but you wish to get their original mapping under the engine, this method is used only under Mac, default=false
#endif

   Bool    ctrlCmd    ()C {return APPLE ? _win  : _ctrl ;} // if any Ctrl is on (on platforms other than Apple), and if any Command is on (on Apple platforms)
   Bool    winCtrl    ()C {return APPLE ? _ctrl : _win  ;} // if any Win  is on (on platforms other than Apple), and if any Control is on (on Apple platforms)
   CChar8* ctrlCmdName()C {return APPLE ? "Cmd" : "Ctrl";}
   CChar8* winCtrlName()C {return APPLE ? "Ctrl": "Win" ;}

#if !EE_PRIVATE
private:
#endif
   Bool      _ctrl, _shift, _alt, _win, _hidden, _swapped_ctrl_cmd, _text_input, _visible, _refresh_visible, _imm, _imm_candidate_hidden, _exclusive;
   Byte      _button[256], _key_buffer_pos, _key_buffer_len;
   Char8     _key_char[256];
   Key       _key_buffer[256];
   KB_KEY    _qwerty[256];
   Int       _cur, _last, _imm_cursor, _last_key_scan_code;
   Flt       _curh_t, _curh_tn;
   Dbl       _last_t;
   RectI     _recti;
   VecI2     _imm_selection;
   Str       _imm_buffer;
   Memc<Str> _imm_candidate, _imm_candidate_temp;
   CChar8   *_key_name[256];
#if EE_PRIVATE
   #if WINDOWS_OLD
      IDirectInputDevice8 *_did;
   #else
      Ptr    _did;
   #endif
   PLATFORM(HIMC, Ptr) _imc;
#else
   Ptr       _did, _imc;
#endif

	Keyboard();
   NO_COPY_CONSTRUCTOR(Keyboard);
}extern
   Kb;
/******************************************************************************/
enum KBSC_FLAG // Keyboard Shortcut Flags
{
   KBSC_CTRL =0x01, // Control required
   KBSC_SHIFT=0x02, // Shift   required (this is ignored for KBSC_CHAR, instead please specify upper/lower case of the character)
   KBSC_ALT  =0x04, // Alt     required (for KBSC_CHAR - only left alt is checked, also for KBSC_CHAR this may not work well if KBSC_CTRL is enabled too, as on Windows pressing Ctrl+Alt triggers accented characters and regular characters may not get generated)
   KBSC_WIN  =0x08, // Win     required

   KBSC_CTRL_CMD=0x10, // Control required on non-Apple platforms, and Command required on Apple platforms
   KBSC_WIN_CTRL=0x20, // Win     required on non-Apple platforms, and Control required on Apple platforms

   KBSC_REPEAT=0x40, // allow repeated execution when holding key for a long time (currently this affects only KBSC_KEY, as KBSC_CHAR always allow this)
};
enum KBSC_MODE : Byte // Keyboard Shortcut Mode
{
   KBSC_NONE, // disabled
   KBSC_CHAR, // treat 'index' as Char
   KBSC_KEY , // treat 'index' as KB_KEY
};
struct KbSc // Keyboard Shortcut
{
   Byte      flag ; // KBSC_FLAG
   KBSC_MODE mode ; // mode
   UShort    index; // Char/KB_KEY depending on 'mode'

   // get
   Bool is    ()C {return mode!=KBSC_NONE;} // if shortcut is valid
   Bool pd    ()C; // if pushed in this frame
   Str  asText()C; // get text describing the shortcut

   // operations
   void eat()C; // eat this shortcut keys input from this frame so it will not be processed by the remaining codes in frame

#if EE_PRIVATE
   Bool testFlag    ()C;
   Bool testFlagChar()C;
#endif

   KbSc(                       ) {T.index=0           ; T.flag=0   ; T.mode=KBSC_NONE;}
   KbSc(Char8  c  , Byte flag=0) {T.index=Char8To16(c); T.flag=flag; T.mode=KBSC_CHAR;}
   KbSc(Char   c  , Byte flag=0) {T.index=c           ; T.flag=flag; T.mode=KBSC_CHAR;}
   KbSc(KB_KEY key, Byte flag=0) {T.index=key         ; T.flag=flag; T.mode=KBSC_KEY ;}
};
/******************************************************************************/
#if EE_PRIVATE
#if WINDOWS_OLD
   #define KB_RAW_INPUT 1
#elif MAC
extern const KB_KEY ScanCodeToQwertyKey[0x80]; extern KB_KEY ScanCodeToKey[Elms(ScanCodeToQwertyKey)];
#elif LINUX
extern const KB_KEY ScanCodeToQwertyKey[0x90]; extern KB_KEY ScanCodeToKey[Elms(ScanCodeToQwertyKey)];
#endif
#endif
/******************************************************************************/
