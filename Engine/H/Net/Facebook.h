/******************************************************************************

   Please read following instructions carefully in order to know how to enable Facebook support.

   In order to use 'Facebook' class, you need to:

      1. Create a "Facebook App" on https://developers.facebook.com/apps/ with exact same name as your Esenthel Application
         1. Click on the App
         2. Select "Settings" tab on the left side
         3. Click "Add Platform"
         4. Select "Android" and "iOS"
         5. Set "Package Name" (Android) and "Budle ID" (iOS) to be the same as your Esenthel Application (this can be viewed by double-clicking on Application in Esenthel Editor)
         6. Enable "Single Sign On" option for both platforms
         7. For "iOS Platform" set "iPhone/iPad Store ID" to your App "Apple ID" (this can be viewed from https://itunesconnect.apple.com/ "Manage Your Apps\Your App\App Information\Identifiers\Apple ID")
         8. For "Android Platform" set "Key Hashes" from:
            1. In Esenthel Editor, open Editor options "M\Editor Options"
            2. Go to "Certificates" tab
            3. Click on "Get Android Key Hash for Facebook"
            4. Paste that Key Hash to the "Key Hashes" field on the Facebook App Settings page
         9. Click "Save Changes" on the bottom of the Facebook App page
      2. Set your Facebook App ID, obtained from https://developers.facebook.com/apps/ to Esenthel Application properties (by double-clicking on Application in Esenthel Editor)
      3. For Android you need to compile your app in "Release" mode (for iOS it doesn't matter), as only "Release" apps are signed with your certificate, which hash we've set on the Facebook page
      4. Once you will be releasing your application to public, you also need to make your "Facebook App" available to the public on the https://developers.facebook.com/apps/ website under the "Status & Review" - field "Do you want to make this app and all its live features available to the general public?"

/******************************************************************************/
#define FACEBOOK (ANDROID || IOS) // if Facebook is supported on this platform
/******************************************************************************/
struct Facebook
{
   struct User
   {
      ULong id  ; // user id
      Str   name; // user name (first+last)

      void clear() {id=0; name.clear();}
   };
   struct UserEmail : User
   {
      Str email; // user email

      void clear() {User::clear(); email.clear();}
   };

#if EE_PRIVATE
   // !! these enums must be equal to "EsenthelActivity.java" !!
#endif
   enum RESULT
   {
      POST_ERROR        , // there was an error while trying to post
      POST_CANCEL       , // user canceled posting
      POST_SUCCESS      , // post completed successfully
      POST_NOT_LOGGED_IN, // user is not logged in to Facebook, and the result of the post is unknown
   };

   void (*callback)(RESULT result); // pointer to a custom function that will be called with processed events, 'result'=message received at the moment

   // manage
   Bool      loggedIn()C; // if currently logged in to Facebook
   Facebook& logIn   () ; // log in  to   Facebook
   Facebook& logOut  () ; // log out from Facebook

   Facebook& getMe     (); // request my         profile information to be downloaded, 'logIn' will be automatically called if needed
   Facebook& getFriends(); // request my friends profile information to be downloaded, 'logIn' will be automatically called if needed

 C UserEmail & me     ()C {return _me     ;} // get my         profile information, 'getMe'      should be called first
 C Mems<User>& friends()C {return _friends;} // get my friends profile information, 'getFriends' should be called first

   Str userImageURL(ULong user_id, Int width=-1, Int height=-1)C; // get user image url from which you can download his/her photo, for example by using the 'Download' class, 'width height'=custom dimensions of the image in pixels -1..Inf (-1=default), you can leave both as default, or specify only one and the other will be set automatically, or you can specify both dimensions, they are treated as hints only

   // operations
   void openPage(C Str &page_name, C Str &page_id=S); // open a Facebook page, 'page_name'=name of the page (for example "EsenthelEngine"), 'page_id'=ID number of the page (for example "161038147263508", this is optional). By default the page will be opened in a browser, however on Android and iOS, if you specify the 'page_id', then it will be opened in the Facebook app if it's available.

   void post(C Str &message, C Str &url=S, C Str &image_url=S, C Str &title=S, C Str &desc=S, C Str &caption=S); // post message to user's timeline, 'message'=message to post (if left as empty then a dialog will be opened where the user will be able to set his own message, this parameter is currently ignored and the dialog is always opened), most of the parameters are optional (if left empty, then Facebook will set them to default values), 'logIn' will be automatically called if needed

#if !EE_PRIVATE
private:
#endif
   UserEmail   _me;
   Mems<User > _friends;

   Facebook();
}extern
   FB;
/******************************************************************************/
