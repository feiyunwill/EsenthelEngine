/******************************************************************************/
enum BS_FLAG // Button State Flag
{
   BS_ON      =0x01, // on
   BS_PUSHED  =0x02, // pushed
   BS_RELEASED=0x04, // released
   BS_DOUBLE  =0x08, // double clicked
   BS_TAPPED  =0x10, // tapped
#if EE_PRIVATE
   BS_REPEAT  =0x20, // repeat push due to holding the button for a long time
   BS_NOT_ON  =BS_PUSHED|BS_RELEASED|BS_DOUBLE|BS_TAPPED|BS_REPEAT,
#endif
};
inline Bool ButtonOn(Byte x) {return FlagTest(x, BS_ON      );} // if button is on
inline Bool ButtonPd(Byte x) {return FlagTest(x, BS_PUSHED  );} // if button pushed
inline Bool ButtonRs(Byte x) {return FlagTest(x, BS_RELEASED);} // if button released
inline Bool ButtonDb(Byte x) {return FlagTest(x, BS_DOUBLE  );} // if button double clicked
inline Bool ButtonTp(Byte x) {return FlagTest(x, BS_TAPPED  );} // if button tapped
#if EE_PRIVATE
inline Bool ButtonRp(Byte x) {return FlagTest(x, BS_REPEAT  );} // if button repeat pushed
#endif
/******************************************************************************/
enum KB_KEY : Byte // Keyboard Keys
{
   KB_NONE=0,

   KB_0='0',
   KB_1='1',
   KB_2='2',
   KB_3='3',
   KB_4='4',
   KB_5='5',
   KB_6='6',
   KB_7='7',
   KB_8='8',
   KB_9='9',

   KB_A='A',
   KB_B='B',
   KB_C='C',
   KB_D='D',
   KB_E='E',
   KB_F='F',
   KB_G='G',
   KB_H='H',
   KB_I='I',
   KB_J='J',
   KB_K='K',
   KB_L='L',
   KB_M='M',
   KB_N='N',
   KB_O='O',
   KB_P='P',
   KB_Q='Q',
   KB_R='R',
   KB_S='S',
   KB_T='T',
   KB_U='U',
   KB_V='V',
   KB_W='W',
   KB_X='X',
   KB_Y='Y',
   KB_Z='Z',

   KB_F1 =0x70, // F1
   KB_F2 =0x71, // F2
   KB_F3 =0x72, // F3
   KB_F4 =0x73, // F4
   KB_F5 =0x74, // F5
   KB_F6 =0x75, // F6
   KB_F7 =0x76, // F7
   KB_F8 =0x77, // F8
   KB_F9 =0x78, // F9
   KB_F10=0x79, // F10
   KB_F11=0x7A, // F11
   KB_F12=0x7B, // F12

   KB_ESC  =0x1B, // escape
   KB_ENTER=0x0D, // enter
   KB_SPACE=0x20, // space
   KB_BACK =0x08, // backspace
   KB_TAB  =0x09, // tab

#if EE_PRIVATE
   KB_SHIFT=0x10, // left or right shift
   KB_CTRL =0x11, // left or right control
   KB_ALT  =0x12, // left or right alt
#endif
   KB_LCTRL =0xA2, // left  control
   KB_RCTRL =0xA3, // right control
   KB_LSHIFT=0xA0, // left  shift
   KB_RSHIFT=0xA1, // right shift
   KB_LALT  =0xA4, // left  alt
   KB_RALT  =0xA5, // right alt
   KB_LWIN  =0x5B, // left  win
   KB_RWIN  =0x5C, // right win
   KB_MENU  =0x5D, // menu
   KB_FIND  =0xAA, // find

   KB_LEFT =0x25, // left  arrow
   KB_RIGHT=0x27, // right arrow
   KB_UP   =0x26, // up    arrow
   KB_DOWN =0x28, // down  arrow

   KB_INS =0x2D, // insert
   KB_DEL =0x2E, // delete
   KB_HOME=0x24, // home
   KB_END =0x23, // end
   KB_PGUP=0x21, // page up
   KB_PGDN=0x22, // page down

   KB_SUB       =0xBD, // subtract      -_
   KB_EQUAL     =0xBB, // equals        =+
   KB_LBRACKET  =0xDB, // left  bracket [{
   KB_RBRACKET  =0xDD, // right bracket ]}
   KB_SEMICOLON =0xBA, // semicolon     ;:
   KB_APOSTROPHE=0xDE, // apostrophe    '"
   KB_COMMA     =0xBC, // comma         ,<
   KB_DOT       =0xBE, // dot           .>
   KB_SLASH     =0xBF, // slash         /?
   KB_BACKSLASH =0xDC, // backslash     \|
   KB_TILDE     =0xC0, // tilde         `~

   KB_NPDIV  =0x6F, // numpad divide   /
   KB_NPMUL  =0x6A, // numpad multiply *
   KB_NPSUB  =0x6D, // numpad subtract -
   KB_NPADD  =0x6B, // numpad add      +
   KB_NPDEL  =0x6E, // numpad del
   KB_NPENTER=0x0D, // numpad enter

   KB_NP0=0x60, // numpad 0
   KB_NP1=0x61, // numpad 1
   KB_NP2=0x62, // numpad 2
   KB_NP3=0x63, // numpad 3
   KB_NP4=0x64, // numpad 4
   KB_NP5=0x65, // numpad 5
   KB_NP6=0x66, // numpad 6
   KB_NP7=0x67, // numpad 7
   KB_NP8=0x68, // numpad 8
   KB_NP9=0x69, // numpad 9

   KB_CAPS  =0x14, // caps   lock
   KB_NUM   =0x90, // num    lock
   KB_SCROLL=0x91, // scroll lock
   KB_PRINT =0x2C, // print  screen
   KB_PAUSE =0x13, // pause  break

   KB_NAV_BACK   =0xA6, // navigate backward
   KB_NAV_FORWARD=0xA7, // navigate  forward

   KB_VOL_DOWN=0xAE, // volume -
   KB_VOL_UP  =0xAF, // volume +
   KB_VOL_MUTE=0xAD, // mute

   KB_MEDIA_PREV=0xB1, // media previous
   KB_MEDIA_NEXT=0xB0, // media next
   KB_MEDIA_PLAY=0xB3, // media play/pause
   KB_MEDIA_STOP=0xB2, // media stop

   KB_ZOOM_IN =0xFB, // zoom in
   KB_ZOOM_OUT=0xFC, // zoom out

#if EE_PRIVATE
   KB_OEM_102=0xE2,
#endif

   // shorter names
   KB_LBR   =KB_LBRACKET  ,
   KB_RBR   =KB_RBRACKET  ,
   KB_SEMI  =KB_SEMICOLON ,
   KB_APO   =KB_APOSTROPHE,
   KB_BSLASH=KB_BACKSLASH ,
};
/******************************************************************************/
#if EE_PRIVATE
#if 0
enum VirtualKey : Byte
{
   VirtualKey_None	= 0,
   VirtualKey_LeftButton	= 1,
   VirtualKey_RightButton	= 2,
   VirtualKey_Cancel	= 3,
   VirtualKey_MiddleButton	= 4,
   VirtualKey_XButton1	= 5,
   VirtualKey_XButton2	= 6,
   VirtualKey_Back	= 8,
   VirtualKey_Tab	= 9,
   VirtualKey_Clear	= 12,
   VirtualKey_Enter	= 13,
   VirtualKey_Shift	= 16,
   VirtualKey_Control	= 17,
   VirtualKey_Menu	= 18,
   VirtualKey_Pause	= 19,
   VirtualKey_CapitalLock	= 20,
   VirtualKey_Kana	= 21,
   VirtualKey_Hangul	= 21,
   VirtualKey_Junja	= 23,
   VirtualKey_Final	= 24,
   VirtualKey_Hanja	= 25,
   VirtualKey_Kanji	= 25,
   VirtualKey_Escape	= 27,
   VirtualKey_Convert	= 28,
   VirtualKey_NonConvert	= 29,
   VirtualKey_Accept	= 30,
   VirtualKey_ModeChange	= 31,
   VirtualKey_Space	= 32,
   VirtualKey_PageUp	= 33,
   VirtualKey_PageDown	= 34,
   VirtualKey_End	= 35,
   VirtualKey_Home	= 36,
   VirtualKey_Left	= 37,
   VirtualKey_Up	= 38,
   VirtualKey_Right	= 39,
   VirtualKey_Down	= 40,
   VirtualKey_Select	= 41,
   VirtualKey_Print	= 42,
   VirtualKey_Execute	= 43,
   VirtualKey_Snapshot	= 44,
   VirtualKey_Insert	= 45,
   VirtualKey_Delete	= 46,
   VirtualKey_Help	= 47,
   VirtualKey_Number0	= 48,
   VirtualKey_Number1	= 49,
   VirtualKey_Number2	= 50,
   VirtualKey_Number3	= 51,
   VirtualKey_Number4	= 52,
   VirtualKey_Number5	= 53,
   VirtualKey_Number6	= 54,
   VirtualKey_Number7	= 55,
   VirtualKey_Number8	= 56,
   VirtualKey_Number9	= 57,
   VirtualKey_A	= 65,
   VirtualKey_B	= 66,
   VirtualKey_C	= 67,
   VirtualKey_D	= 68,
   VirtualKey_E	= 69,
   VirtualKey_F	= 70,
   VirtualKey_G	= 71,
   VirtualKey_H	= 72,
   VirtualKey_I	= 73,
   VirtualKey_J	= 74,
   VirtualKey_K	= 75,
   VirtualKey_L	= 76,
   VirtualKey_M	= 77,
   VirtualKey_N	= 78,
   VirtualKey_O	= 79,
   VirtualKey_P	= 80,
   VirtualKey_Q	= 81,
   VirtualKey_R	= 82,
   VirtualKey_S	= 83,
   VirtualKey_T	= 84,
   VirtualKey_U	= 85,
   VirtualKey_V	= 86,
   VirtualKey_W	= 87,
   VirtualKey_X	= 88,
   VirtualKey_Y	= 89,
   VirtualKey_Z	= 90,
   VirtualKey_LeftWindows	= 91,
   VirtualKey_RightWindows	= 92,
   VirtualKey_Application	= 93,
   VirtualKey_Sleep	= 95,
   VirtualKey_NumberPad0	= 96,
   VirtualKey_NumberPad1	= 97,
   VirtualKey_NumberPad2	= 98,
   VirtualKey_NumberPad3	= 99,
   VirtualKey_NumberPad4	= 100,
   VirtualKey_NumberPad5	= 101,
   VirtualKey_NumberPad6	= 102,
   VirtualKey_NumberPad7	= 103,
   VirtualKey_NumberPad8	= 104,
   VirtualKey_NumberPad9	= 105,
   VirtualKey_Multiply	= 106,
   VirtualKey_Add	= 107,
   VirtualKey_Separator	= 108,
   VirtualKey_Subtract	= 109,
   VirtualKey_Decimal	= 110,
   VirtualKey_Divide	= 111,
   VirtualKey_F1	= 112,
   VirtualKey_F2	= 113,
   VirtualKey_F3	= 114,
   VirtualKey_F4	= 115,
   VirtualKey_F5	= 116,
   VirtualKey_F6	= 117,
   VirtualKey_F7	= 118,
   VirtualKey_F8	= 119,
   VirtualKey_F9	= 120,
   VirtualKey_F10	= 121,
   VirtualKey_F11	= 122,
   VirtualKey_F12	= 123,
   VirtualKey_F13	= 124,
   VirtualKey_F14	= 125,
   VirtualKey_F15	= 126,
   VirtualKey_F16	= 127,
   VirtualKey_F17	= 128,
   VirtualKey_F18	= 129,
   VirtualKey_F19	= 130,
   VirtualKey_F20	= 131,
   VirtualKey_F21	= 132,
   VirtualKey_F22	= 133,
   VirtualKey_F23	= 134,
   VirtualKey_F24	= 135,
   VirtualKey_NavigationView	= 136,
   VirtualKey_NavigationMenu	= 137,
   VirtualKey_NavigationUp	= 138,
   VirtualKey_NavigationDown	= 139,
   VirtualKey_NavigationLeft	= 140,
   VirtualKey_NavigationRight	= 141,
   VirtualKey_NavigationAccept	= 142,
   VirtualKey_NavigationCancel	= 143,
   VirtualKey_NumberKeyLock	= 144,
   VirtualKey_Scroll	= 145,
   VirtualKey_LeftShift	= 160,
   VirtualKey_RightShift	= 161,
   VirtualKey_LeftControl	= 162,
   VirtualKey_RightControl	= 163,
   VirtualKey_LeftMenu	= 164,
   VirtualKey_RightMenu	= 165,
   VirtualKey_GoBack	= 166,
   VirtualKey_GoForward	= 167,
   VirtualKey_Refresh	= 168,
   VirtualKey_Stop	= 169,
   VirtualKey_Search	= 170,
   VirtualKey_Favorites	= 171,
   VirtualKey_GoHome	= 172,
   VirtualKey_GamepadA	= 195,
   VirtualKey_GamepadB	= 196,
   VirtualKey_GamepadX	= 197,
   VirtualKey_GamepadY	= 198,
   VirtualKey_GamepadRightShoulder	= 199,
   VirtualKey_GamepadLeftShoulder	= 200,
   VirtualKey_GamepadLeftTrigger	= 201,
   VirtualKey_GamepadRightTrigger	= 202,
   VirtualKey_GamepadDPadUp	= 203,
   VirtualKey_GamepadDPadDown	= 204,
   VirtualKey_GamepadDPadLeft	= 205,
   VirtualKey_GamepadDPadRight	= 206,
   VirtualKey_GamepadMenu	= 207,
   VirtualKey_GamepadView	= 208,
   VirtualKey_GamepadLeftThumbstickButton	= 209,
   VirtualKey_GamepadRightThumbstickButton	= 210,
   VirtualKey_GamepadLeftThumbstickUp	= 211,
   VirtualKey_GamepadLeftThumbstickDown	= 212,
   VirtualKey_GamepadLeftThumbstickRight	= 213,
   VirtualKey_GamepadLeftThumbstickLeft	= 214,
   VirtualKey_GamepadRightThumbstickUp	= 215,
   VirtualKey_GamepadRightThumbstickDown	= 216,
   VirtualKey_GamepadRightThumbstickRight	= 217,
   VirtualKey_GamepadRightThumbstickLeft	= 218
};

#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03
#define VK_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */
#define VK_XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
#define VK_XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */
#define VK_BACK           0x08
#define VK_TAB            0x09
#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D
#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14

#define VK_KANA           0x15
#define VK_HANGEUL        0x15  /* old name - should be here for compatibility */
#define VK_HANGUL         0x15
#define VK_JUNJA          0x17
#define VK_FINAL          0x18
#define VK_HANJA          0x19
#define VK_KANJI          0x19

#define VK_ESCAPE         0x1B

#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F

#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F
#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D
#define VK_SLEEP          0x5F
#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F

#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87

#define VK_NAVIGATION_VIEW     0x88
#define VK_NAVIGATION_MENU     0x89
#define VK_NAVIGATION_UP       0x8A
#define VK_NAVIGATION_DOWN     0x8B
#define VK_NAVIGATION_LEFT     0x8C
#define VK_NAVIGATION_RIGHT    0x8D
#define VK_NAVIGATION_ACCEPT   0x8E
#define VK_NAVIGATION_CANCEL   0x8F

#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91

#define VK_OEM_NEC_EQUAL  0x92   // '=' key on numpad
#define VK_OEM_FJ_JISHO   0x92   // 'Dictionary' key
#define VK_OEM_FJ_MASSHOU 0x93   // 'Unregister word' key
#define VK_OEM_FJ_TOUROKU 0x94   // 'Register word' key
#define VK_OEM_FJ_LOYA    0x95   // 'Left OYAYUBI' key
#define VK_OEM_FJ_ROYA    0x96   // 'Right OYAYUBI' key

#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

#define VK_BROWSER_BACK        0xA6
#define VK_BROWSER_FORWARD     0xA7
#define VK_BROWSER_REFRESH     0xA8
#define VK_BROWSER_STOP        0xA9
#define VK_BROWSER_SEARCH      0xAA
#define VK_BROWSER_FAVORITES   0xAB
#define VK_BROWSER_HOME        0xAC

#define VK_VOLUME_MUTE         0xAD
#define VK_VOLUME_DOWN         0xAE
#define VK_VOLUME_UP           0xAF
#define VK_MEDIA_NEXT_TRACK    0xB0
#define VK_MEDIA_PREV_TRACK    0xB1
#define VK_MEDIA_STOP          0xB2
#define VK_MEDIA_PLAY_PAUSE    0xB3
#define VK_LAUNCH_MAIL         0xB4
#define VK_LAUNCH_MEDIA_SELECT 0xB5
#define VK_LAUNCH_APP1         0xB6
#define VK_LAUNCH_APP2         0xB7

#define VK_OEM_1          0xBA   // ';:' for US
#define VK_OEM_PLUS       0xBB   // '+' any country
#define VK_OEM_COMMA      0xBC   // ',' any country
#define VK_OEM_MINUS      0xBD   // '-' any country
#define VK_OEM_PERIOD     0xBE   // '.' any country
#define VK_OEM_2          0xBF   // '/?' for US
#define VK_OEM_3          0xC0   // '`~' for US

#define VK_GAMEPAD_A                         0xC3
#define VK_GAMEPAD_B                         0xC4
#define VK_GAMEPAD_X                         0xC5
#define VK_GAMEPAD_Y                         0xC6
#define VK_GAMEPAD_RIGHT_SHOULDER            0xC7
#define VK_GAMEPAD_LEFT_SHOULDER             0xC8
#define VK_GAMEPAD_LEFT_TRIGGER              0xC9
#define VK_GAMEPAD_RIGHT_TRIGGER             0xCA
#define VK_GAMEPAD_DPAD_UP                   0xCB
#define VK_GAMEPAD_DPAD_DOWN                 0xCC
#define VK_GAMEPAD_DPAD_LEFT                 0xCD
#define VK_GAMEPAD_DPAD_RIGHT                0xCE
#define VK_GAMEPAD_MENU                      0xCF
#define VK_GAMEPAD_VIEW                      0xD0
#define VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON    0xD1
#define VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON   0xD2
#define VK_GAMEPAD_LEFT_THUMBSTICK_UP        0xD3
#define VK_GAMEPAD_LEFT_THUMBSTICK_DOWN      0xD4
#define VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT     0xD5
#define VK_GAMEPAD_LEFT_THUMBSTICK_LEFT      0xD6
#define VK_GAMEPAD_RIGHT_THUMBSTICK_UP       0xD7
#define VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN     0xD8
#define VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT    0xD9
#define VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT     0xDA

#define VK_OEM_4          0xDB  //  '[{' for US
#define VK_OEM_5          0xDC  //  '\|' for US
#define VK_OEM_6          0xDD  //  ']}' for US
#define VK_OEM_7          0xDE  //  ''"' for US
#define VK_OEM_8          0xDF
#define VK_OEM_AX         0xE1  //  'AX' key on Japanese AX kbd
#define VK_OEM_102        0xE2  //  "<>" or "\|" on RT 102-key kbd.
#define VK_ICO_HELP       0xE3  //  Help key on ICO
#define VK_ICO_00         0xE4  //  00 key on ICO
#define VK_PROCESSKEY     0xE5
#define VK_ICO_CLEAR      0xE6
#define VK_PACKET         0xE7

#define VK_OEM_RESET      0xE9
#define VK_OEM_JUMP       0xEA
#define VK_OEM_PA1        0xEB
#define VK_OEM_PA2        0xEC
#define VK_OEM_PA3        0xED
#define VK_OEM_WSCTRL     0xEE
#define VK_OEM_CUSEL      0xEF
#define VK_OEM_ATTN       0xF0
#define VK_OEM_FINISH     0xF1
#define VK_OEM_COPY       0xF2
#define VK_OEM_AUTO       0xF3
#define VK_OEM_ENLW       0xF4
#define VK_OEM_BACKTAB    0xF5

#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE

#endif
#endif
/******************************************************************************/
