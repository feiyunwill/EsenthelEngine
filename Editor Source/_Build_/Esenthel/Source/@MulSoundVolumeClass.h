/******************************************************************************/
/******************************************************************************/
class MulSoundVolumeClass : PropWin
{
   flt       volume;
   Button    ok;
   Memc<UID> elms;

   static void OK(MulSoundVolumeClass &smc);

   void create();
   void display(C MemPtr<UID> &elms);

   virtual void update(C GuiPC &gpc)override;

public:
   MulSoundVolumeClass();
};
/******************************************************************************/
/******************************************************************************/
extern MulSoundVolumeClass MulSoundVolume;
/******************************************************************************/
