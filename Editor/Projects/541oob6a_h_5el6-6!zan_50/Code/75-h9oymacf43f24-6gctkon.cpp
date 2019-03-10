/******************************************************************************/
class Item : Game.Item // extend items to include item Icons, and parameters
{
   Str       name;
   ITEM_TYPE type=ITEM_MISC;
   ImagePtr  icon;

   // manage
   void create(Object &obj)
   {
      super.create(obj);

      // set name
      if(Param *p=obj.findParam("name"))name=p.asText();

      // set type
      if(Param *p=obj.findParam("type"))type=p.asEnum(ITEM_MISC);

      // get icon
      if(Param *p=obj.findParam("icon"))icon=p.asID();
   }

   // draw
   virtual uint drawPrepare()
   {
      if(Lit==this)SetHighlight(Color(48, 48, 48, 0)); uint modes=super.drawPrepare();
      if(Lit==this)SetHighlight();
      return modes;
   }
   void drawIcon(C Vec2 &pos)
   {
      if(icon)
      {
         icon->drawFit(Rect_LU(pos.x, pos.y, icon->w()*PIXEL_SIZE, icon->h()*PIXEL_SIZE));
      }
   }

   // io
   virtual bool save(File &f)
   {
      if(super.save(f))
      {
         f<<type<<name<<icon.id();
         return f.ok();
      }
      return false;
   }
   virtual bool load(File &f)
   {
      if(super.load(f))
      {
         f>>type>>name; icon=f.getUID();
         if(f.ok())return true;
      }
      return false;
   }
}
/******************************************************************************/
