/******************************************************************************

   Use 'PlatformStore' to communicate with the Platform Store, like:
      Microsoft Windows Store
      Google    Play    Store for Android
      Apple     App     Store for iOS

   In Google Play in addition to your own item ID's, you can use the following:
      "android.test.purchased"        - test purchase that will always succeed
      "android.test.canceled"         - test purchase that will always fail
      "android.test.item_unavailable" - test purchase that will always fail due to item unavailability
      "android.test.refunded"         - test purchase that will trigger item refund

   In Google Play to test your custom item ID's, make sure to run the Application:
      -in Release mode (which will force signing of the APK file)
      -on actual device (not on emulator)
      -with the same Package Name as in the store
   Otherwise testing purchases will fail with following message:
      "This version of the application is not configured for billing through Google Play"

   In Apple Store to test your custom item ID's, make sure to run the Application:
      -on actual device (not on simulator)
      -with the same Package Name as in the store
      -have your bank account details setup on iTunes Connect
      -sometimes you may need to wait a few hours after creating new items in iTunes Connect until they become available
      -make sure that the items in iTunes Connect are "Cleared for Sale"

/******************************************************************************/
#define STORE (WINDOWS_NEW || ANDROID || APPLE) // if Store is supported on this platform
/******************************************************************************/
struct PlatformStore // class allowing to communicate with Platform Store
{
#if EE_PRIVATE
   // !! These enums must be equal to "EsenthelActivity.java" !!
#endif
   enum RESULT
   {
      PURCHASED          , // item was purchased successfully, 'purchases' container will now     include it
      CONSUMED           , // item was consumed  successfully, 'purchases' container will now not include it
      REFUND             , // item was refunded by the store
      WAITING            , // result is not yet known as the command was forwarded further, the result will be passed to the 'callback' function (if specified) at a later time
      USER_CANCELED      , // user    canceled purchase
      SERVICE_CANCELED   , // service canceled purchase, possibly due to lack of funds or other payment failure
      SERVICE_UNAVAILABLE, // service is unavailable
      ITEM_UNAVAILABLE   , // item was not found in the store
      ALREADY_OWNED      , // item is already owned and can't be purchased again
      NOT_OWNED          , // item is not     owned and can't be consumed
      VERIFICATION_FAIL  , // item purchase didn't pass the verification test based on the key provided in "Application Android License Key"
      UNKNOWN            , // unknown error
      REFRESHED_ITEMS    , // 'items'     were refreshed, this can occur after calling the 'refreshItems'     method
      REFRESHED_PURCHASES, // 'purchases' were refreshed, this can occur after calling the 'refreshPurchases' method
   };

   struct Item
   {
      Bool subscription; // if this is a subscription or a regular item (valid only in Google Play)
      Str  id   , // ID
           name , // name
           desc , // description
           price; // price
   };

   struct Purchase
   {
      Str        id, // ID of the item
               data, // custom string that was specified in 'buy' method (valid only in Google Play)
              token; // token of purchase, you can use it to consume this purchase
      DateTime date; // date  of purchase in UTC time zone
   };

   void (*callback)(RESULT result, C Purchase *purchase); // pointer to a custom function that will be called with processed events, 'result'=message received at the moment, 'purchase'=purchase data related to this message (it may be null if not available, also some of its members may be empty)

   // get
   Bool supportsItems        ()C {return _supports_items;} // if store supports purchasing regular items
   Bool supportsSubscriptions()C {return _supports_subs ;} // if store supports purchasing subscriptions

 C Memc<Item    >& items    ()C {return _items    ;} // get all known item details that were requested using 'refreshItems'
 C Memc<Purchase>& purchases()C {return _purchases;} // get all active purchases, you need to call 'refreshPurchases' first for this to have any

 C Item    * findItem    (C Str &item_id)C; // find item     from the 'items'     container by its  ID, null on fail
 C Purchase* findPurchase(C Str &item_id)C; // find purchase from the 'purchases' container by item ID, null on fail
#if EE_PRIVATE
 C Purchase* findPurchaseByToken(C Str &token)C; // find purchase from the 'purchases' container by its token, null on fail
   void           update();
   Bool backgroundUpdate();
#endif

   // operations
   Bool refreshItems    (C MemPtr<Str> &item_ids); // refresh 'items'     container for selected items, once the latest data has been received, 'callback' function will be called with REFRESHED_ITEMS     result, false on fail, 'item_ids'=item ID's for which you want to refresh the details
   Bool refreshPurchases(                       ); // refresh 'purchases' container                   , once the latest data has been received, 'callback' function will be called with REFRESHED_PURCHASES result, false on fail

   RESULT buy(C Str &id, Bool subscription=false, C Str &data=S); // purchase an item, 'id'=ID of the item, 'subscription'=if it's a subscription or regular item (this is required only for Google Play, for Apple Store it's ignored), 'data'=custom string that will be attached to the purchase and later accessible through 'Purchase.data' (used only in Google Play, in Apple Store this is ignored)
   RESULT buy(C Item &item, C Str &data=S) {return buy(item.id, item.subscription, data);}

   RESULT consume(C Str &token); // consume purchase, 'token'=item purchase token ('Purchase.token' which can be obtained from the 'purchases' container)
   RESULT consume(C Purchase &purchase) {return consume(purchase.token);}

   PlatformStore& restorePurchases(); // this function is usable only for Apple, it will call "[[SKPaymentQueue defaultQueue] restoreCompletedTransactions]"

#if !EE_PRIVATE
private:
#endif
   struct Processed : Purchase {RESULT result;};
   Bool            _supports_items, _supports_subs, _has_new_purchases, _refresh_purchases;
   Memc<Item     > _items, _new_items;
   Memc<Purchase > _purchases, _new_purchases;
   Memc<Processed> _processed;
   Thread          _thread;
   SyncLock        _lock;
   Memc<Str      > _get_item_details, _consume;

   PlatformStore();
  ~PlatformStore();
}extern
   Store; // access 'PlatformStore' through this singleton object
/******************************************************************************/
