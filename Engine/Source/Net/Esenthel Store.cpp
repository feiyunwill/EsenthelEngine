/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
static StrO MD5Text(C Str8 &text)
{
   UID id=MD5Mem(text(), text.length()); return TextHexMem(&id, SIZE(id));
}
void EsenthelStore::RegisterAccount()
{
   Explore("http://www.esenthel.com/?id=store&mode=register");
}
EsenthelStore::RESULT EsenthelStore::GetAccessKey(Int item_id)
{
   if(item_id<=0)return INVALID_ITEM;
   return Explore(S+"http://www.esenthel.com/?id=store&get_access_key="+item_id) ? CONNECTING : CANT_CONNECT;
}
EsenthelStore::RESULT EsenthelStore::Buy(C Str &email, Int app_id, PURCHASE_TYPE purchase_type)
{
   if(!ValidEmail(email))return INVALID_EMAIL_FORMAT;
   if(  app_id       <=0)return INVALID_ITEM;
   Int item_id;
   switch(purchase_type)
   {
      default          : return INVALID_ITEM;
      case PURCHASE_1  : item_id=140; break;
      case PURCHASE_2  : item_id=141; break;
      case PURCHASE_3  : item_id=142; break;
      case PURCHASE_5  : item_id=143; break;
      case PURCHASE_10 : item_id=144; break;
      case PURCHASE_20 : item_id=145; break;
      case PURCHASE_50 : item_id=146; break;
      case PURCHASE_100: item_id=147; break;
   }
   return Explore(S+"http://www.esenthel.com/store.php?cmd=buy_item&i="+item_id+"&a="+app_id+"&u="+email) ? CONNECTING : CANT_CONNECT;
}
Int EsenthelStore::PurchaseToUSD(PURCHASE_TYPE purchase_type)
{
   switch(purchase_type)
   {
      case PURCHASE_1  : return   1;
      case PURCHASE_2  : return   2;
      case PURCHASE_3  : return   3;
      case PURCHASE_5  : return   5;
      case PURCHASE_10 : return  10;
      case PURCHASE_20 : return  20;
      case PURCHASE_50 : return  50;
      case PURCHASE_100: return 100;
   }
   return 0;
}
/******************************************************************************/
EsenthelStore::EsenthelStore()
{
  _device_id=false;
  _license_result=_purchase_result=NONE;
  _license_item_id=_purchase_item_id=0;
  _license_r=_purchase_r=0;
}
/******************************************************************************/
// LICENSE TEST
/******************************************************************************/
void EsenthelStore::licenseClear(Bool params)
{
  _license_download.del(); // delete this first in case it would change anything
  _device_id=false;
  _license_result=NONE;
  _license_r=0;
   if(params){_license_item_id=0; _license_key.del(); _license_email.del();}
}
void EsenthelStore::licenseTest(Int item_id, C Str &license_key, C Str &email, Bool device_id)
{
   licenseClear(false); T._license_key=license_key; T._license_email=email; // don't clear 'license_key' member in case it's the 'license_key' parameter

   if(item_id<=0)_license_result=INVALID_ITEM;else
   if(license_key.is() && !ValidLicenseKey(license_key))_license_result=INVALID_LICENSE_KEY_FORMAT;else
   if(email      .is() && !ValidEmail     (email      ))_license_result=INVALID_EMAIL_FORMAT;else
   if(license_key.is() || email.is() || device_id) // if we were actually requested to test anything
   {
      T._license_item_id=item_id;
      T._license_r      =Random();
      T._device_id      =device_id;
      Memt<HTTPParam> params;
    //Int time=DateTime().getUTC().seconds1970()/(60*5); // 5 mins
                          params.New().set("i"  , S+item_id); // item id
                          params.New().set("r"  , S+_license_r); // request id
      if(license_key.is())params.New().set("l"  , license_key); // license key
      if(email      .is())params.New().set("e"  , email); // email
      if(device_id       )params.New().set("c"  , DeviceID()); // computer/device id
                          params.New().set("cmd", "test_license"); // command
     _license_download.create("http://www.esenthel.com/test_license.php", params);
     _license_result=CONNECTING;
   }
}
void EsenthelStore::updateLicense()
{
   if(_license_result==CONNECTING)switch(_license_download.state()) // check for progress
   {
      case DWNL_NONE :
      case DWNL_ERROR: _license_download.del(); _license_result=CANT_CONNECT; break;

      case DWNL_DONE:
      {
         RESULT   r=INVALID_RESPONSE;
         FileText f; f.readMem(_license_download.data(), _license_download.size());
         TextData data; if(data.load(f))if(C TextNode *result=data.findNode("Result"))switch(result->asInt())
         {
            case 1: r=LICENSE_KEY_FAIL; break;
            case 2: r=  DEVICE_ID_FAIL; break;
            case 3: r= EMAIL_NOT_FOUND; break;
            case 0: if(C TextNode *confirm=data.findNode("Confirm")) // OK
            {
               Int  time=DateTime().getUTC().seconds1970()/(60*5); // 5 mins
               Str8 str=CaseDown(_license_key+_license_email+(_device_id ? DeviceID() : S)+_license_item_id+_license_r),
                    a=str+(time-1), b=str+time, c=str+(time+1); // time tolerance
               UID  code_id; code_id.fromHex(confirm->asText());
               r=((MD5Mem(a(), a.length())==code_id || MD5Mem(b(), b.length())==code_id || MD5Mem(c(), c.length())==code_id) ? OK : CONFIRM_CODE_FAIL);
            }break;
         }
        _license_download.del(); _license_result=r; // set at the end
      }break;
   }
}
EsenthelStore::RESULT EsenthelStore::licenseResult() {updateLicense(); return _license_result;}
/******************************************************************************/
// PURCHASES
/******************************************************************************/
void EsenthelStore::updatePurchases()
{
   if(_purchase_result==CONNECTING)switch(_purchase_download.state()) // check for progress
   {
      case DWNL_NONE :
      case DWNL_ERROR: _purchase_download.del(); _purchase_result=CANT_CONNECT; break;

      case DWNL_DONE:
      {
         RESULT   r=INVALID_RESPONSE;
         FileText f; f.readMem(_purchase_download.data(), _purchase_download.size());
         TextData data; if(data.load(f))if(TextNode *result=data.findNode("result"))switch(result->asInt())
         {
            case 1: r=EMAIL_NOT_FOUND ; break;
            case 2: r=INVALID_PASSWORD; break;
            case 3: r=INVALID_CALL    ; break;
            case 0: if(C TextNode *confirm=data.findNode("Confirm")) // OK
            {
               r=((MD5Text(CaseDown(purchasesEmail())+'|'+_purchase_r+'|'+_purchase_item_id)==confirm->asText()) ? OK : CONFIRM_CODE_FAIL);
               if(TextNode *purchases=data.findNode("purchases"))FREPA(purchases->nodes) // list in order
               {
                  TextNode &purchase=purchases->nodes[i];
                  if(TextNode *date       =purchase.findNode("Date"))
                  if(TextNode *license_key=purchase.findNode("LicenseKey"))
                  if(TextNode *cost       =purchase.findNode("Cost"))
                  {
                     Int c=cost->asInt(); if(c>=99)
                     {
                        Purchase  &purchase=_purchases.New();
                                   purchase.date.fromText(date->asText());
                                   purchase.id  =  license_key->asText() ;
                        if(c>=9999)purchase.type=PURCHASE_100;else
                        if(c>=4999)purchase.type=PURCHASE_50 ;else
                        if(c>=1999)purchase.type=PURCHASE_20 ;else
                        if(c>= 999)purchase.type=PURCHASE_10 ;else
                        if(c>= 499)purchase.type=PURCHASE_5  ;else
                        if(c>= 299)purchase.type=PURCHASE_3  ;else
                        if(c>= 199)purchase.type=PURCHASE_2  ;else
                                   purchase.type=PURCHASE_1  ;
                     }
                  }
               }
            }break;
         }
        _purchase_download.del(); _purchase_result=r; // set at the end
      }break;
   }

   REPA(_consume)
   {
      Download &consume=_consume[i]; switch(consume.state())
      {
         case DWNL_DONE:
         {
            FileText f; f.readMem(consume.data(), consume.size());
            TextData data; if(data.load(f))if(TextNode *result=data.findNode("result"))switch(result->asInt())
            {
               case 3: break; // invalid Key
               case 2: break; // invalid PurchaseID
               case 1: break; // email not found
               case 0: // removed OK
               {
                  if(TextNode *purchase_id=data.findNode("Purchase"))
                     REPA(_purchases)if(_purchases[i].id==purchase_id->value)_purchases.remove(i, true);
               }break;
            }
         } // !! no break on purpose so it can be removed !!
         case DWNL_NONE :
         case DWNL_ERROR: _consume.removeValid(i); break;
      }
   }
}
void EsenthelStore::purchasesClear()
{
  _purchase_download.del(); // delete this first in case it would change anything
  _purchase_item_id=0;
  _purchase_r=0;
  _purchase_email.clear();
  _purchase_result=NONE;
  _purchases.clear();
}
       EsenthelStore::RESULT     EsenthelStore::purchasesResult() {updatePurchases(); return _purchase_result;}
C Memc<EsenthelStore::Purchase>& EsenthelStore::purchases      () {updatePurchases(); return _purchases;}

void EsenthelStore::purchasesRefresh(C Str &email, C Str &access_key, Int item_id)
{
   purchasesClear(); _purchase_email=email; _purchase_item_id=item_id;
   if(!ValidEmail(email)      )_purchase_result=INVALID_EMAIL_FORMAT;else
   if( access_key.length()!=32)_purchase_result=INVALID_PASSWORD    ;else
   if( item_id            <= 0)_purchase_result=INVALID_ITEM        ;else
   {
      Memt<HTTPParam> params;
     _purchase_r=Random(1, INT_MAX);
      params.New().set("cmd", "get");
      params.New().set("i", S+item_id);
      params.New().set("r", S+_purchase_r); // request_id
      params.New().set("e", email);
      params.New().set("k", MD5Text(CaseDown(access_key)+'|'+_purchase_r+'|'+item_id));
     _purchase_download.create("http://www.esenthel.com/purchases.php", params);
     _purchase_result=CONNECTING;
   }
}
void EsenthelStore::_purchasesRefresh(C Str &email, C Str &password, Int item_id)
{
   purchasesClear(); _purchase_email=email; _purchase_item_id=item_id;
   if(!ValidEmail(email))_purchase_result=INVALID_EMAIL_FORMAT;else
   if(!password.is()    )_purchase_result=INVALID_PASSWORD    ;else
   if( item_id       <=0)_purchase_result=INVALID_ITEM        ;else
   {
      Memt<HTTPParam> params;
     _purchase_r=Random(1, INT_MAX);
      params.New().set("cmd", "get2");
      params.New().set("i", S+item_id);
      params.New().set("r", S+_purchase_r); // request_id
      params.New().set("e", email);
      params.New().set("k", MD5Text(CaseDown(password)+'|'+_purchase_r+'|'+item_id));
     _purchase_download.create("http://www.esenthel.com/purchases.php", params);
     _purchase_result=CONNECTING;
   }
}
EsenthelStore::RESULT EsenthelStore::consume(C Str &email, Int item_id, C Str &purchase_id)
{
   if(!ValidEmail(email))return INVALID_EMAIL_FORMAT;
   if( item_id<=0       )return INVALID_ITEM;
   if(!purchase_id.is() )return INVALID_ITEM;
   Memt<HTTPParam> params;
   Int request_id=Random(1, INT_MAX);
   params.New().set("cmd", "consume");
   params.New().set("i", S+item_id);
   params.New().set("r", S+request_id);
   params.New().set("e", email);
   params.New().set("p", purchase_id);
   params.New().set("k", MD5Text(CaseDown(purchase_id)+'|'+request_id+'|'+item_id));
  _consume.New().create("http://www.esenthel.com/purchases.php", params);
   return CONNECTING;
}
/******************************************************************************/
}
/******************************************************************************/
