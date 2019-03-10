/******************************************************************************/
class ChatMessage // a single message in a chat box
{
   Str   text;
   Color background=TRANSPARENT;
   flt   height=0, y_min=0, y_max=0;

   // get / set
   void set(C Str &text, int color_index) {T.text=text; T.background=ColorHSB(color_index*0.3, 0.1, 1);}

   // draw
   void draw(TextStyle &ts, C Vec2 &box_top_left, flt box_width, flt margin)
   {
      Rect_LU rect(box_top_left+Vec2(0, -y_min), box_width, height);
      rect.draw(background);
      rect.min.x+=margin;
      D.text(ts, rect, text, AUTO_LINE_SPACE);
   }
}
/******************************************************************************/
class ChatMessages : GuiCustom
{
   flt               margin=0.01;
   TextStyle         ts;
   Memc<ChatMessage> messages;

   // manage
   ChatMessages& create()
   {
      super.create();

      ts.reset(true);
      ts.size=0.05;
      ts.align.set(1, -1);

      return T;
   }

   // operate
   void New(C Str &text) // add new message
   {
      messages.New().set(text, messages.elms());
      rebuildSpace();
   }
   void rebuildSpace()
   {
      flt y=0, width=0;

      Region *region=null;
      if(parent() && parent().type()==GO_REGION)region=&parent().asRegion();

      if(region)width=region.clientWidth()-margin;

      flt line_height=ts.lineHeight();
      FREPA(messages)
      {
         flt h=ts.textLines(messages[i].text, width, AUTO_LINE_SPACE)*line_height;

         messages[i].height=h  ;
         messages[i].y_min =y  ;
         messages[i].y_max =y+h;

         y+=h;
      }

      if(region)region.virtualSize(&Vec2(0, y));
   }

   // draw
   static int FindMinY(C ChatMessage &message, C flt &min_y) {return Compare(message.y_max, min_y);}
   static int FindMaxY(C ChatMessage &message, C flt &max_y) {return Compare(message.y_min, max_y);}

   virtual void draw(C GuiPC &gpc) // draw object
   {
      if(visible() && gpc.visible && parent() && parent().type()==GO_REGION)
      {
         // clip drawing rectangle
         D.clip(gpc.clip);

         // calculate offsets
         Region &region       =parent().asRegion();
         flt     offset       =region.slidebar[1].offset();  // get regions vertical slidebar offset
         Vec2    box_top_left =gpc.offset;                   // get top left corner in "screen space"
         flt     box_width    =region.clientWidth(),         // get visible space width
                 visible_min_y=offset,                       // get visible space in "message space"
                 visible_max_y=region.clientHeight()+offset; // get visible space in "message space"

         // find visible message indexes
         int from, to;
         messages.binarySearch(visible_min_y, from, FindMinY),
         messages.binarySearch(visible_max_y, to  , FindMaxY);

         // draw them
         for(int i=from; i<to; i++)messages[i].draw(ts, box_top_left, box_width, margin);
      }
   }
}
/******************************************************************************/
Window       window; // gui  window
Region       region; // gui  region
Button       add   ; // add  message button
ChatMessages chat  ; // chat messages
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
Str texts[3]=
{
   "This is a random text",
   "This is a random text but a little bit longer",
   "This is a random text but much much longer, hopefully to take much more more more more more more more more lines",
};
void AddMessage(ptr)
{
   bool at_end=region.slidebar[1].wantedAtEnd(0.02);
   chat.New("New Custom Message");
   if(at_end)region.scrollToY(9999); // scroll to the end of the region
}
bool Init()
{
   Gui   +=window.create(Rect(-0.5 , -0.4, 0.5 ,  0.4), "Chatbox");
   window+=region.create(Rect( 0.05, -0.6, 0.95, -0.1));
   window+=add   .create(Rect_C(window.clientWidth()/2, -0.67, 0.3, 0.06), "Add Message").func(AddMessage);
   region+=chat  .create();

   FREP(20)chat.New(S+i+": "+texts[Random(Elms(texts))]);

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Gui.update();
   return true;
}
/******************************************************************************/
void Draw()
{
   D  .clear(TURQ);
   Gui.draw ();
}
/******************************************************************************/
