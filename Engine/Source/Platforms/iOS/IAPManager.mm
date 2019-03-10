/******************************************************************************/
#include "iOS.h"
/******************************************************************************/
namespace EE{
/******************************************************************************/
static Bool        HasPurchases;
static DateTime    AppStartTime=DateTime().getUTC();
       IAPManager *IAPMgr;
/******************************************************************************/
}
/******************************************************************************/
@implementation IAPManager
/******************************************************************************/
-(void)productsRequest:(SKProductsRequest*)request didReceiveResponse:(SKProductsResponse*)response
{
   NSNumberFormatter *nf=[[NSNumberFormatter alloc] init];
   [nf setFormatterBehavior:NSNumberFormatterBehavior10_4];
   [nf setNumberStyle:NSNumberFormatterCurrencyStyle];
   REP([response.products count])
   {
      SKProduct *product=[response.products objectAtIndex:i];
      [nf setLocale:product.priceLocale];
      NSString *price=[nf stringFromNumber:product.price];
      Str item_id=product.productIdentifier;
      PlatformStore::Item *item=ConstCast(Store.findItem(item_id)); if(!item)item=&Store._items.New();
      item->subscription=false;
      item->id   =item_id;
      item->name =product.localizedTitle;
      item->desc =product.localizedDescription;
      item->price=price;
    //[price release]; we're not the owner so we can't release it
   }
   for(NSString *invalid in response.invalidProductIdentifiers)Store._items.removeData(Store.findItem(AppleString(invalid)), true);
   [nf      release];
   [request release]; // release the request which we've created manually in 'Store.refreshItems'
   if(Store.callback)Store.callback(PlatformStore::REFRESHED_ITEMS, null);
}
-(void)paymentQueue:(SKPaymentQueue*)queue updatedTransactions:(NSArray*)transactions;
{
   Bool response_from_server=false;
   for(SKPaymentTransaction *transaction in transactions)
   {
      if(transaction.transactionState!=SKPaymentTransactionStatePurchasing)
      {
         response_from_server=true;
         SKPaymentTransaction *org_trans=((transaction.transactionState==SKPaymentTransactionStateRestored) ? transaction.originalTransaction : transaction);
         PlatformStore::RESULT    result=PlatformStore::PURCHASED;
         PlatformStore::Purchase purchase;
         purchase.date .from(org_trans.transactionDate); // use the original transaction
         purchase.id   =transaction.payment.productIdentifier;
         purchase.token=transaction.transactionIdentifier;
         if(transaction.transactionState==SKPaymentTransactionStateFailed)
         {
            switch(transaction.error.code)
            {
               case SKErrorPaymentCancelled        : result=PlatformStore::USER_CANCELED   ; if(Store.findPurchase(purchase.id)){result=PlatformStore::ALREADY_OWNED; break;} break; // 'SKErrorPaymentCancelled' also stands for when we already own the item, so do these checks
            #if IOS
               case SKErrorStoreProductNotAvailable: result=PlatformStore::ITEM_UNAVAILABLE; break;
            #endif
               case SKErrorPaymentNotAllowed       : result=PlatformStore::SERVICE_CANCELED; break;
               case SKErrorClientInvalid           : result=PlatformStore::SERVICE_CANCELED; break;
               default                             : result=PlatformStore::UNKNOWN         ; break;
            }
            [[SKPaymentQueue defaultQueue] finishTransaction:transaction]; // automatically remove failed transactions
         }

         // first add to the list of purchases
         if(result==PlatformStore::PURCHASED && !Store.findPurchaseByToken(purchase.token))Store._purchases.add(purchase);

         // now call the callback
         if(Store.callback && (purchase.date>AppStartTime || result!=PlatformStore::PURCHASED || HasPurchases)) // this may receive purchases from the past, for which we don't want callback to get notified, so verify that its date is newer than when the app started (since failed purchases have no date, then list all failed too, also list it if we've already received purchases from the server)
         {
            HasPurchases=true; // if this was our first purchase, then set 'HasPurchases' to true, so 'REFRESHED_PURCHASES' won't be called right after that
            Store.callback(result, &purchase); // !! here don't pass purchase from '_purchases' !!
         }
      }
   }
   if(response_from_server && !HasPurchases) // if this was a response from server, then we can assume that we now know all of the transactions for this app
   {
      HasPurchases=true;
      if(Store.callback)Store.callback(PlatformStore::REFRESHED_PURCHASES, null);
   }
}
/******************************************************************************/
@end
/******************************************************************************/
