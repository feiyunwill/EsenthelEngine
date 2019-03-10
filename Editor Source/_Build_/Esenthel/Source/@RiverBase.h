/******************************************************************************/
/******************************************************************************/
class RiverBase
{
   class Vtx
   {
      flt radius;
      Vec pos;

      void set(flt radius, C Vec &pos);

public:
   Vtx();
   };

   bool        removed;
   byte        smooth;
   flt         depth;
   Vec2        tex_scale;
   UID         material;
   Memc<Vtx  > vtxs;
   Memc<VecI2> edges;
   TimeStamp   removed_time, smooth_time, depth_time, tex_scale_time, material_time, vtx_edge_time;

   // get
   bool equal(C RiverBase &src)C;
   bool newer(C RiverBase &src)C;

   bool getRect(Rect &rect)C;

   uint memUsage()C;

   // operations
   void setSmooth  (byte    smooth   );
   void setDepth   (flt     depth    );
   void setTexScale(C Vec2 &tex_scale);

   bool sync(C RiverBase &src);
   bool undo(C RiverBase &src);

   // draw
   void draw(C Color &color, C Color &depth_color=TRANSPARENT)C;

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);

public:
   RiverBase();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
