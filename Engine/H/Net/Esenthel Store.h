/******************************************************************************

   Use 'EsenthelStore' to communicate with Esenthel Store.
      You can use it for example if you've developed a game which is sold through Esenthel Store,
      and in the game you'd like to verify if it's a licensed copy.

      This testing can be done for example in following way:
         1. Ask the user for license key
         2. Test that license key (optionally with Device ID of the user's device)
         3. Allow to play the game only based on successful response from Esenthel Store

   'EsenthelStore' class can also be used for making In-App purchases.

/******************************************************************************/
struct EsenthelStore // class allowing to communicate with Esenthel Store
{
   enum RESULT
   {
      NONE                      , // nothing was ordered yet
      INVALID_ITEM              , // 'item_id' is invalid
      INVALID_LICENSE_KEY_FORMAT, // 'license_key' was specified however it is not of the "XXXXX-XXXXX-XXXXX-XXXXX-XXXXX" format
      INVALID_EMAIL_FORMAT      , // 'email'       was specified however it is not of correct format
      INVALID_PASSWORD          , // 'password'    is invalid, its length must be between 4..16 characters and may not contain unicode characters
      INVALID_NAME              , // 'name'        is invalid, its length must not exceed 32 characters
      INVALID_CALL              , // '_purchasesRefresh' was called
      CONNECTING                , // connecting to Esenthel Store at the moment
      CANT_CONNECT              , // could not connect to Esenthel Store
      INVALID_RESPONSE          , // received an invalid response from Esenthel Store
      DATABASE_ERROR            , // Esenthel Store replied that it couldn't connect to the DataBase
      EMAIL_NOT_FOUND           , // Esenthel Store replied that given email was not found
      EMAIL_ALREADY_EXISTS      , // Esenthel Store replied that given email is already in use
      LICENSE_KEY_FAIL          , // Esenthel Store replied that given license is not valid for the specified item
      DEVICE_ID_FAIL            , // Esenthel Store replied that given license is     valid for the specified item however 'DeviceID' did not match   the one in the store
      CONFIRM_CODE_FAIL         , // Esenthel Store replied that given license is     valid for the specified item and     'DeviceID'         matches the one in the store however confirmation code was incorrect (this may be caused by user having invalid time/date/timezone on his device)
      OK                        , // Esenthel Store replied that given license is     valid for the specified item and     'DeviceID'         matches the one in the store and     confirmation code was   correct
   };

   static void RegisterAccount(); // this function will open Esenthel Store website in a System Browser for the purpose of registering a new account there

   // LICENSE VERIFICATION
      static Str DeviceID() {return EE::DeviceID(true).asHex();} // get Device ID used in Esenthel Store, you can use this function to get the ID of current device and display it to the users, so they can set it for their products in Esenthel Store if needed

      // operations
      void licenseClear(Bool params=true); // cancel any current 'licenseTest' request and clear the 'licenseResult' to NONE, 'params'=if additionally clear the last specified parameters (such as 'licenseItemID', 'licenseKey' and 'licenseEmail')
      void licenseTest (Int  item_id, C Str &license_key=S, C Str &email=S, Bool device_id=false); // test if license is ok for a given item in Esenthel Store, 'item_id'=ID of an item in Esenthel Store (if you're the author of the item, then you can get its ID from Esenthel Store), following parameters are optional, only one of them needs to be specified (for example you can test only license key, only email, only device id, or multiple at the same time), 'license_key'=if specified (must be of "XXXXX-XXXXX-XXXXX-XXXXX-XXXXX" format) then the license key will be tested if it's valid for the specified item in Esenthel Store, 'email'=test if there exists a user account with specified email address which has a valid license key for specified item, 'device_id'=test if Device ID of current machine matches the one assigned to the license key, once this function is called it will try to connect to Esenthel Store on a secondary thread to verify the data, while it's running you can investigate the 'licenseResult' method to get the results

      // get
      RESULT licenseResult(); // get result of the 'licenseTest'
      Int    licenseItemID()C {return _license_item_id;} // get item    id  that was specified in the last 'licenseTest' request
    C Str&   licenseKey   ()C {return _license_key    ;} // get license key that was specified in the last 'licenseTest' request
    C Str&   licenseEmail ()C {return _license_email  ;} // get email       that was specified in the last 'licenseTest' request

   // IN-APP PURCHASES
      enum PURCHASE_TYPE : Byte
      {
         PURCHASE_1  , //  0.99 USD In-App Purchase
         PURCHASE_2  , //  1.99 USD In-App Purchase
         PURCHASE_3  , //  2.99 USD In-App Purchase
         PURCHASE_5  , //  4.99 USD In-App Purchase
         PURCHASE_10 , //  9.99 USD In-App Purchase
         PURCHASE_20 , // 19.99 USD In-App Purchase
         PURCHASE_50 , // 49.99 USD In-App Purchase
         PURCHASE_100, // 99.99 USD In-App Purchase
      };
      struct Purchase
      {
         Str           id  ; // unique ID of this purchase
         PURCHASE_TYPE type; // purchase type
         DateTime      date; // date of purchase in UTC time zone
      };

      static RESULT GetAccessKey(Int item_id); // this function will open Esenthel Store website in a System Browser and display to the user an "Access Key", which he can copy to the System Clipboard and then use it inside your Apps for the 'purchasesRefresh' method. That "Access Key" works like a password (however it is not the same as user's account password), allowing the app to operate on In-App purchases, 'item_id'=ID of an item in Esenthel Store (if you're the author of the item, then you can get its ID from Esenthel Store)

      static RESULT Buy(C Str &email, Int item_id, PURCHASE_TYPE purchase_type); // this function will open Esenthel Store website in a System Browser for the purpose of making a purchase, 'email'=Esenthel Store account email, 'item_id'=ID of an item in Esenthel Store (if you're the author of the item, then you can get its ID from Esenthel Store), 'purchase_type'=type of the purchase

      static Int PurchaseToUSD(PURCHASE_TYPE purchase_type); // convert 'purchase_type' into a USD amount rounded to the nearest integer, PURCHASE_2 -> 2, PURCHASE_3 -> 3, PURCHASE_5 -> 5, ..

      void            purchasesClear ();                           // cancel any current 'purchasesRefresh' request, clear 'purchasesResult' to NONE and clear 'purchases'
      RESULT          purchasesResult();                           // get result of 'purchasesRefresh'
    C Memc<Purchase>& purchases      ();                           // get all active purchases, you need to call 'purchasesRefresh' first for this to have any
    C Str           & purchasesEmail ()C {return _purchase_email;} // get email that was specified in the last 'purchasesRefresh' request

      void   purchasesRefresh(C Str &email, C Str &access_key, Int item_id); // refresh the list of purchases by contacting Esenthel Store, 'email'=Esenthel Store account email for the owner of the purchases, 'access_key'=access key allowing to contact Esenthel Store (THIS IS NOT Esenthel Store account password !! access key must be obtained using 'GetAccessKey' function), 'item_id'=ID of an item in Esenthel Store (if you're the author of the item, then you can get its ID from Esenthel Store), after calling this method you can monitor 'purchasesResult' for progress about the refresh, list of purchases will be available using the 'purchases' method
      RESULT consume         (C Str &email,                    Int item_id, C Str      &purchase_id);                                               // consume a specific purchase, 'email'=Esenthel Store account email for the owner of the purchases, 'item_id'=ID of an item in Esenthel Store (if you're the author of the item, then you can get its ID from Esenthel Store), 'purchase_id'=unique ID of the purchase to be consumed (this is 'Purchase.id'), once this method is called, Esenthel Store will be contacted with the request of removing the purchase from its database, upon success, the purchase will be automatically removed from the 'purchases' list, this method may fail if Internet connection was lost/unavailable
      RESULT consume         (C Str &email,                    Int item_id, C Purchase &purchase   ) {return consume(email, item_id, purchase.id);} // consume a specific purchase, 'email'=Esenthel Store account email for the owner of the purchases, 'item_id'=ID of an item in Esenthel Store (if you're the author of the item, then you can get its ID from Esenthel Store), 'purchase_id'=unique ID of the purchase to be consumed (this is 'Purchase.id'), once this method is called, Esenthel Store will be contacted with the request of removing the purchase from its database, upon success, the purchase will be automatically removed from the 'purchases' list, this method may fail if Internet connection was lost/unavailable

   EsenthelStore();

   // DO NOT USE the following method, as IT WILL NOT WORK for your In-App purchases
   void _purchasesRefresh(C Str &email, C Str &password, Int item_id);

#if !EE_PRIVATE
private:
#endif
   Bool           _device_id;
   Int            _license_item_id, _purchase_item_id;
   UInt           _license_r, _purchase_r;
   RESULT         _license_result, _purchase_result;
   Str            _license_key, _license_email, _purchase_email;
   Download       _license_download, _purchase_download;
   Memx<Download> _consume;
   Memc<Purchase> _purchases;

#if EE_PRIVATE
   void updateLicense  ();
   void updatePurchases();
#endif
};
/******************************************************************************/
