/******************************************************************************

   Use 'Ms' to access Mouse input.

/******************************************************************************/
struct Mouse // Mouse Input
{
 C Vec2&      pos()C {return _pos      ;}   void pos(C Vec2 &pos); // get/set cursor position                      (in Screen Coordinates)
 C Vec2& startPos()C {return _start_pos;}                          // get     cursor position of first button push (in Screen Coordinates), this     is equal to the most recent cursor position at the moment of first button push - "bp(0)"
 C Vec2&      d  ()C {return _delta    ;}                          // get     cursor position delta                (in Screen Coordinates), delta    is not affected by mouse clipping or  display scale, use this for smooth unlimited mouse movement deltas (for example rotate the player)
 C Vec2&      dc ()C {return _delta_clp;}                          // get     cursor position delta clipped        (in Screen Coordinates), delta    is     affected by mouse clipping and display scale, use this for 2D gui object movement, limited by mouse cursor position
 C Vec2&      vel()C {return _vel      ;}                          // get     cursor velocity                      (in Screen Coordinates), velocity is not affected by mouse clipping or  display scale, it's calculated based on few last positions

   Byte state      (Int x)C {return InRange(x, _button) ?          _button[x]  :     0;} // get button BS_FLAG state
   Bool b          (Int x)C {return InRange(x, _button) ? ButtonOn(_button[x]) : false;} // if  button 'x' is on
   Bool bp         (Int x)C {return InRange(x, _button) ? ButtonPd(_button[x]) : false;} // if  button 'x' pushed   in this frame
   Bool br         (Int x)C {return InRange(x, _button) ? ButtonRs(_button[x]) : false;} // if  button 'x' released in this frame
   Bool bd         (Int x)C {return InRange(x, _button) ? ButtonDb(_button[x]) : false;} // if  button 'x' double clicked
   Bool tapped     (Int x)C {return InRange(x, _button) ? ButtonTp(_button[x]) : false;} // if  button 'x' tapped, tapping is a single quick push and release of the button without any movement, this can be true at the moment of the release with the condition that there was no movement and the push life was very short
   Bool tappedFirst(Int x)C {return tapped(x) && _first                               ;} // if  tapped which was caused by first click of a double-click, double-clicks generate two taps, you can use this method to detect only the first one

   Flt wheel ()C {return _wheel.y;} // get vertical   mouse wheel delta
   Flt wheelX()C {return _wheel.x;} // get horizontal mouse wheel delta

   Int wheelI ()C {return _wheel_i.y;} // get vertical   mouse wheel delta in integer steps
   Int wheelIX()C {return _wheel_i.x;} // get horizontal mouse wheel delta in integer steps

   Dbl startTime()C {return                _start_time;} // get time of when the latest button was pushed, obtained using "Time.appTime()"
   Flt life     ()C {return Time.appTime()-_start_time;} // get how long     the latest button is  pushed

   Bool selecting()C {return _selecting;} // if enough                     movement occurred since the latest button was pushed to consider it selecting
   Bool dragging ()C {return _dragging ;} // if enough time has passed and movement occurred since the latest button was pushed to consider it dragging

 C VecI2&  windowPos()C {return  _window_posi;} // cursor position in Application Window  in Pixel Coordinates
 C VecI2& desktopPos()C {return _desktop_posi;} // cursor position in System      Desktop in Pixel Coordinates
 C VecI2& pixelDelta()C {return       _deltai;} // cursor position delta                  in Pixel Coordinates

   Bool detected()C {return _detected;} // if mouse was detected in the system

   Flt speed()C;   void speed(Flt speed); // get/set mouse movement speed, this affects only 'Ms.d', default=1

   SMOOTH_VALUE_MODE smooth(                      )C {return _sv_delta.mode(    );} // get position delta smoothing, default=SV_WEIGHT4
   void              smooth(SMOOTH_VALUE_MODE mode)  {       _sv_delta.mode(mode);} // set position delta smoothing

   CChar8* buttonName(Int x)C; // get button name, buttonName(0) -> "Mouse1", buttonName(1) -> "Mouse2", ..

   // test if mouse cursor is in rectangle
   Bool test(C Rect &rect)C {return Cuts(pos(), rect);}

   // cursor movement clipping
   Mouse& clip  (C Rect *rect=null, Int window=-1); // clip mouse cursor to given rectangle, 'window'=if additionally clip to the application window client rectangle (-1=don't change, 0=off, 1=on)
   Mouse& freeze(                                ); // call this in each frame when you want to freeze the mouse cursor position

   // cursor visuals
#if EE_PRIVATE
   void resetVisibility();
#endif
   Bool   visible(            )C {return  _visible          ;} // if     cursor is visible
   Bool   hidden (            )C {return !_visible          ;} // if     cursor is hidden
   Mouse& visible(Bool visible);                               // set    cursor visibility
   Mouse& toggle (            )  {return visible(!visible());} // toggle cursor visibility
   Mouse& show   (            )  {return visible(true      );} // show   cursor
   Mouse& hide   (            )  {return visible(false     );} // hide   cursor
   Mouse& cursor (C ImagePtr &image, C VecI2 &hot_spot=VecI2(0, 0), Bool hardware=true, Bool reset=false); // set cursor image, 'hot_spot'=point in image coordinates, 'hardware'=if use hardware cursor (this allows to draw the mouse cursor with full display speed, regardless of the game speed), 'reset'=if reset cursor even if the parameters are same as before

   // operations
   void eat     (          ); // eat any button  input from this frame so it will not be processed by the remaining codes in frame
   void eat     (Int button); // eat    'button' input from this frame so it will not be processed by the remaining codes in frame
   void eatWheel(          ); // eat    'wheel'  input from this frame so it will not be processed by the remaining codes in frame

#if EE_PRIVATE
   // manage
   void del    ();
   void create ();
   void acquire(Bool on);

   // operations
   void resetCursor();
   void clear      ();
   void push       (Byte b, Flt double_click_time=DoubleClickTime);
   void release    (Byte b);
   void update     ();
   void clipUpdate ();
   void draw       ();
#endif

#if !EE_PRIVATE
private:
#endif
   Byte             _button[8];
   Bool             _selecting, _dragging, _first, _detected, _on_client, _visible, _freezed, _clip_rect_on, _clip_window, _freeze, _action, _want_cur_hw, _locked;
   Int              _cur;
   Flt              _speed;
   Dbl              _start_time, _wheel_time;
   Vec2             _pos, _delta, _delta_clp, _delta_relative, _vel, _start_pos, _wheel, _wheel_f;
   VecI2            _window_posi, _desktop_posi, _deltai, _hot_spot, _wheel_i;
   Rect             _clip_rect;
   SmoothValue2     _sv_delta;
   SmoothValueTime2 _sv_vel;
   ImagePtr         _image;
   CChar8          *_button_name[8];
#if EE_PRIVATE
   #if WINDOWS_OLD
      IDirectInputDevice8 *_did;
   #else
      Ptr                  _did;
   #endif
#else
   Ptr              _did;
#endif

   Mouse();
   NO_COPY_CONSTRUCTOR(Mouse);
}extern
   Ms;
/******************************************************************************/
#define MOUSE_IMAGE_SIZE 0.05f
/******************************************************************************/
