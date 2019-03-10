/******************************************************************************

   Use 'MiniMap' to automatically manage world mini maps.

/******************************************************************************/
namespace Game{
/******************************************************************************/
struct MiniMap
{
   struct Settings
   {
      Int areas_per_image, image_size;
      Flt area_size;

      Bool save(C Str &name)C; // save to   file, false on fail
      Bool load(C Str &name) ; // load from file, false on fail

      Bool save(File &f)C; // save to   file, false on fail
      Bool load(File &f) ; // load from file, false on fail

      void del();
      Settings() {del();}
   };

   // load
   void operator=(C Str  &name); // load mini map from specified 'name', Exit  on fail
   void operator=(C UID  &id  ); // load mini map from specified 'id'  , Exit  on fail
   Bool load     (C Str  &name); // load mini map from specified 'name', false on fail
   Bool load     (C UID  &id  ); // load mini map from specified 'id'  , false on fail

   // get
 C UID& id           ()C {return _id                      ;} // get name ID         of current mini map
 C Str& name         ()C {return _name                    ;} // get name            of current mini map
   Flt  areaSize     ()C {return _settings.area_size      ;} // get size of an area of current mini map (in meters)
   Int  areasPerImage()C {return _settings.areas_per_image;} // get number of areas per single image map

   Image& operator()(C VecI2 &image_xy) {return *_map(image_xy);} // get map image, 'image_xy'=image coordinates (these are not Area coordinates! many areas can be stored in a single map image)

   // convert coordinates
      // world (meters) <-> area
      Vec2 worldToArea (C Vec2 &pos )C {return pos /areaSize();} // convert world 'pos' position to area coordinates
      Vec2  areaToWorld(C Vec2 &area)C {return area*areaSize();} // convert area coordinates     to world position

      // area <-> image
      Vec2  areaToImage(C Vec2 &area )C {return area /areasPerImage();} // convert area coordinates to image
      Vec2 imageToArea (C Vec2 &image)C {return image*areasPerImage();} // convert image            to area coordinates

      // world (meters) <-> image
      Vec2 worldToImage(C Vec2 &pos  )C {return pos  /(areaSize()*areasPerImage());} // convert world 'pos' position to image
      Vec2 imageToWorld(C Vec2 &image)C {return image*(areaSize()*areasPerImage());} // convert image                to world position

   // operations
   void clear(C RectI *leave=null); // remove all loaded images from memory, 'leave'=coordinates of images to leave in memory (without removing), if 'leave'==null then all images are removed

   void del(); // delete manually
   MiniMap();

#if !EE_PRIVATE
private:
#endif
   Settings          _settings;
   Str               _name;
   UID               _id;
   Map<VecI2, Image> _map;

#if EE_PRIVATE
   static Bool Create(Image &image, C VecI2 &key, Ptr user);
#endif
};
/******************************************************************************/
} // namespace
/******************************************************************************/
