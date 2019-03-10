/******************************************************************************/
   template<int i>   Str  GuiView::TabsTabText(C Tabs &tabs          ) {return InRange(i, tabs) ? tabs.tab(i).text( ) : S;}
   template<int i>   void GuiView::TabsTabText(  Tabs &tabs, C Str &t) {    if(InRange(i, tabs))  tabs.tab(i).text(t);    }
/******************************************************************************/
