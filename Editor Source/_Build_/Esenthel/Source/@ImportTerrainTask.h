/******************************************************************************/
/******************************************************************************/
class ImportTerrainTask : Window
{
   bool        height_do, mtrl_do, color_do, color_img_alpha;
   int         height_mode, mtrl_mode, color_mode, res;
   flt         mtrl_blend, color_blend, grid_level;
   Vec2        height;
   VecI2       area_xy;
   RectI       area_rect;
   UID         world_id;
   WorldVer   *world_ver;
   Image       height_image, mtrl_image, color_image;
   MaterialPtr mtrl, mtrl_channels[4];
   Progress    progress;

   static void Stop(ImportTerrainTask &it);

   void create(ImportTerrainClass &it);


   class MaterialIntensities
   {
      class Mtrl
      {
         byte index;
         flt  intensity;
      };

      static int Compare(C Mtrl &a, C Mtrl &b); // we sort from higher to lower order, so swap 'a' 'b' order

      Mtrl mtrl[8]; int mtrls;

      void set(int i, byte &index, flt &intensity);
      void addMtrl(byte index, flt intensity);
      void sort();                          

public:
   MaterialIntensities();
   };
   void import(Heightmap &hm);
   void update();
   bool step();

public:
   ImportTerrainTask();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
