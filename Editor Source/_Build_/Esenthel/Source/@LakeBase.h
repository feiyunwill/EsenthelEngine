/******************************************************************************/
/******************************************************************************

   Lake and River can be saved on MainThread and loaded on BuilderThread.
      Use 'WorldAreaSync' for that.

/******************************************************************************/
class LakeBase
{
   bool            removed;
   flt             depth, tex_scale;
   UID             material;
   Memc<Memc<Vec> > polys;
   TimeStamp       removed_time, depth_time, tex_scale_time, material_time, polys_time;

   // get
   bool equal(C LakeBase &src)C;
   bool newer(C LakeBase &src)C;

   bool getRect(Rect &rect)C;

   uint memUsage()C;

   // operations
   void setDepth(flt depth);  
   bool sync(C LakeBase &src);
   bool undo(C LakeBase &src);

   // draw
   void draw(C Color &color, C Color &depth_color=TRANSPARENT)C;

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);

public:
   LakeBase();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
