/******************************************************************************/
   void  ProjectEx::hide(            )  {visible(false);}
   bool  ProjectEx::visible(            )C {return outer_region.visible(  );}
   void  ProjectEx::visible(  bool  on  )  {       outer_region.visible(on);}
   void  ProjectEx::draw(            )  {       outer_region.draw();}
   C Rect& ProjectEx::rect(            )  {return outer_region.rect();}
   void  ProjectEx::rect(C Rect &rect)  {       outer_region.rect(rect);}
   Vec2  ProjectEx::size(            )  {return outer_region.size();}
/******************************************************************************/
