/******************************************************************************

   You can access an element from cache in 2 ways:
      1. By requesting standard C++ pointer

      or

      2. By requesting specialized 'CacheElmPtr' pointer

   For example when operating on 'Images' cache (which stores 'Image' objects)
      you'll need to do the following:

      1. Image   *image=Images(ID);

      or

      2. ImagePtr image=ID;

   When using the 1st way, the cache will load the image, store it in memory forever (as long as the cache exists),
      and return pointer to loaded object.

   This solution is OK if you don't load many objects, or you have plenty of free memory (RAM).
      However in other case, it may be required that elements are stored in memory only when needed,
      and once they're no longer needed they'll be automatically removed from the memory.
      This is especially needed for systems with low amount of RAM (for example mobile platforms).

   In order to overcome this issue the cache should automatically count by how many pointers an element is referenced,
      and when there are no more pointers referencing an element, the cache will automatically delete the element.

   To automatically count the number of pointers referencing an element 'CacheElmPtr' has been introduced.
      'CacheElmPtr' is a specialized pointer, which can be treated as a replacement for typical C++ pointer, except:
         -when acquiring address of an element   , it automatically increases the element's "reference count"
         -when destroying the specialized pointer, it automatically decreases the element's "reference count"

   A "reference count" is the number of all active 'CacheElmPtr' pointers referencing an object.
      Knowing the "reference count" for each element in the cache, the engine can automatically free the element,
      when there are no more specialized pointers referencing the element.

   Class of specialized pointers has been designed in such way, that it can be used just as typical C++ pointers.

   Please check following tutorial for more informations.

/******************************************************************************/
UID      id_0=UID(1119600675, 1212460399, 80010661, 526665178)   , // ID of one of the images in project data
         id_1=UID(2338406646, 1284858740, 3530955697, 2844784642); // ID of one of the images in project data
Image   *image    ; // define C++ pointer to 'Image' object
ImagePtr image_ptr; // define specialized 'ImagePtr' pointer. 'ImagePtr' is a typedef of 'CacheElmPtr',
                    // which automatically modifies "reference count" for "Image" objects in "Images" cache.
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
Bool Init()
{
   // 1. Image* method
   image=Images(id_0); // access "id_0" image for "Image*", this will keep the "id_0" image forever

   // 2. ImagePtr method
   image_ptr=id_1; // access "id_1" image using "ImagePtr", this will keep the "id_1" only as long there's at least one 'ImagePtr' accessing it

   // additional method of loading elements from cache:
   {
      // you can also use 'ImagePtr' class methods to access elements:
      image_ptr.require(id_1); // this results in the same as "image_ptr=id_1"
   }

   // setting 'CacheElmPtr' to manually allocated data
   {
      // 'CacheElmPtr' can also be used to point to manually allocated data just like C++ pointers:
      Image    custom_image;
      ImagePtr custom_image_ptr=&custom_image; // the 'ImagePtr' will try to increase the "reference count" of the 'Image' object
                                               // however since the image does not belong to 'Images' cache,
                                               // the "reference count" does not exist, and will not be changed.
   }

   // copying 'CacheElmPtr' pointers
   {
      // cache element pointers support copying, so you can do the following:
      ImagePtr image_copy=image_ptr; // now 'image_copy' also points to the same element as 'image_ptr'
                                     // 'ImagePtr' automatically increases/decreses "reference count" of the element when copying
                                     // which means that there are now total of 2 pointers referencing the same image -
                                     // "reference count" of the element is now equal to 2

     //    once 'ImagePtr' objects are destroyed (their destructor is called) they automatically decrease the "reference count" of the element
   } // <- here the 'image_copy' destructor is called, which means that it no longer references the "id_1" image
     //    and the only pointer referencing "id_1" is now 'image_ptr', with a total "reference count" of 1

   return true;
} 
/******************************************************************************/
// What you shouldn't do:
/******************************************************************************/
ImagePtr GetImage()
{
   return id_0; // this is OK
}
void Function2()
{
   {
      ImagePtr image=GetImage(); // this is OK
    //if(image)image->...        // call some method on 'image', this is OK
   }
   {
    //Image   *image=GetImage(); // this is WON'T COMPILE, to prevent automatic casting from 'ImagePtr' to 'Image*'
   }
   {
      Image   *image=GetImage()(); // this is INCORRECT, you can't access an 'ImagePtr' and cast it to 'Image*' without storing the 'ImagePtr' too,
                                   // once 'GetImage' returns 'ImagePtr', and the 'ImagePtr' is not copied to another 'ImagePtr'
                                   // the element can get immediately unloaded

    //if(image)image->...          // this is INCORRECT, 'image' may point to invalid data
   }
   {
      ImagePtr image=GetImage(); // this is OK
      Image   *i    =image   (); // this is OK, since you have a copy of "ImagePtr image" which still references the data
    //if(i)i->...                // call some method on 'i', this is OK
      image=null;                // however once you modify the 'ImagePtr', or it gets deleted (destructor gets called), you may not use the stored 'Image*' again
    //if(i)i->...                // now this is INCORRECT
   }
   
   // In general you may use 'CacheElmPtr' just as regular C++ pointers,
   // but just make sure not to cast it to C++ pointers AND use them after 'CacheElmPtr' is no longer valid (or modified).
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
Bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   if(Ms.bp(0))image    =null; // set 'image    ' to null, this doesn't affect the cache at all, because it's only a C++ pointer
   if(Ms.bp(1))image_ptr=null; // set 'image_ptr' to null, this automatically decreases the "reference count" of an element, and unloads it if there are no more specialized pointers referencing it
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);

   if(image)image->draw(Rect_C(-0.5, 0, 0.5, 0.5)); // draw 'Image*'

   if(image_ptr)image_ptr->draw(Rect_C(0.5, 0, 0.5, 0.5)); // draw 'ImagePtr', you can use its methods just as you would on 'Image*' pointer

   D.text(0, 0.9, S+"Press Left Mouse Button to set 'image' to null,");
   D.text(0, 0.8, S+"and notice that amount of loaded 'Images' will not change.");

   D.text(0, 0.6, S+"Press Right Mouse Button to set 'image_ptr' to null,");
   D.text(0, 0.5, S+"and notice that amount of loaded 'Images' will automatically decrease.");

   D.text(0, -0.9, S+"Number of elements stored in 'Images' cache: "+Images.elms());
}
/******************************************************************************/
