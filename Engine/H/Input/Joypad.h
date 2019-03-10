/******************************************************************************

   Use 'Joypads' container to access Joypads input.

/******************************************************************************/
struct Joypad // Joypad Input
{
   Vec2 dir       , //                   direction
        dir_a  [2], //            analog direction
        dir_an [2]; // normalized analog direction
   Flt  trigger[2]; // trigger

   Bool b (Int x)C {return InRange(x, _button) ? ButtonOn(_button[x]) : false;} // if button 'x' is on
   Bool bp(Int x)C {return InRange(x, _button) ? ButtonPd(_button[x]) : false;} // if button 'x' pushed   in this frame
   Bool br(Int x)C {return InRange(x, _button) ? ButtonRs(_button[x]) : false;} // if button 'x' released in this frame
   Bool bd(Int x)C {return InRange(x, _button) ? ButtonDb(_button[x]) : false;} // if button 'x' double clicked

   Bool supportsVibrations()C; // if supports force feedback vibrations

   UInt         id(     )C {return _id  ;} // get unique id of this joypad
 C Str&       name(     )C {return _name;} // get joypad name
   Str  buttonName(Int x)C;                // get button name, buttonName(0) -> "Joypad1", buttonName(1) -> "Joypad2", ..

   Joypad& vibration(C Vec2 &force); // set force feedback vibrations, (-1,-1)..(1,1), length of vector specifies intensity (value of (0,0) disables vibrations)

#if EE_PRIVATE
   // manage
   void acquire(Bool on);
   void update (C Byte *on, Int elms);
   void update ();
   void clear  ();
   void zero   ();
   void push   (Byte b);
   void release(Byte b);
   Int  index  ()C;
#endif

#if !EE_PRIVATE
private:
#endif
   Byte _button[32], _vibration_axes, _xinput1, _offset_x, _offset_y, _connected;
   Flt  _last_t[32];
   UInt _id;
   Str  _name;
#if EE_PRIVATE
   #if WINDOWS_OLD
      IDirectInputDevice8 *_did;
      IDirectInputEffect  *_effect;
   #else
      Ptr                  _did, _effect;
   #endif
#else
   Ptr  _did, _effect;
#endif

   static CChar *_button_name[32];

  ~Joypad();
   Joypad();

   NO_COPY_CONSTRUCTOR(Joypad);
};
extern MemtN<Joypad, 4> Joypads;
/******************************************************************************/
Joypad* FindJoypad(UInt id); // find joypad in 'Joypads' container according to its 'id', null on fail
#if EE_PRIVATE
void ListJoypads();
void InitJoypads();
void ShutJoypads();
#endif
/******************************************************************************/
