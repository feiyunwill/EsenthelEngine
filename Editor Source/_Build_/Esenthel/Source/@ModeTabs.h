/******************************************************************************/
class ModeTabs : Tabs
{
   class TabEx : Tab
   {
      virtual void update(C GuiPC &gpc)override;
      virtual void draw(C GuiPC &gpc)override;
   };

   Memc<MODE> order;
   Button     close;
   bool       available[MODE_NUM];
   GuiSkinPtr Gui_skin; // this is set to Editor's Gui.skin, and not current App's GuiSkin

   ModeTabs();

   static void CloseActive(ModeTabs &mt );
          void closeActive(bool      all);

   static void Changed(ModeTabs &mt);
          void changed();
   Elm* elm(int i);
   void closeTab(int i, bool all);
   void kbSet();
   void kbToggle();
   void activateLast();
   void create();
   void setCloseVisibility();
   void setTabs();
   virtual Tabs& rect(C Rect &rect)override;   C Rect& rect()C;
   void resize();
   bool tabAvailable(MODE mode         )C;
   void tabAvailable(MODE mode, bool on);
   bool visibleTabs();                 
   Tab* visibleTab(int t);             

   virtual GuiObj& show()override;
   virtual GuiObj& hide()override;

   virtual void update(C GuiPC &gpc)override;
   virtual void draw(C GuiPC &gpc)override;
};
/******************************************************************************/
/******************************************************************************/
extern ModeTabs Mode;
/******************************************************************************/
