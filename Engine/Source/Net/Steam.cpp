/******************************************************************************/
#include "stdafx.h"

#define SUPPORT_STEAM (DESKTOP && !ARM)
#define CLOUD_WORKAROUND 1 // perhaps this is needed only when running apps manually and not through Steam

#if SUPPORT_STEAM
   #pragma warning(push)
   #pragma warning(disable:4996)
   #include "../../../ThirdPartyLibs/begin.h"
   #include "../../../ThirdPartyLibs/SteamWorks/steam_api.h"
   #include "../../../ThirdPartyLibs/end.h"
   #pragma warning(pop)
#endif

#if DEBUG && 0
   #define ISteamMicroTxn "ISteamMicroTxnSandbox"
#else
   #define ISteamMicroTxn "ISteamMicroTxn"
#endif

#if SUPPORT_MBED_TLS
   #define STEAM_API "https://api.steampowered.com/" ISteamMicroTxn // use HTTPS if we can support it
#else
   #define STEAM_API  "http://api.steampowered.com/" ISteamMicroTxn // fall back to HTTP
#endif

#define DAYS_PER_MONTH 30.436875f // average number of days in a month (365.2425 days in a year / 12 months) https://en.wikipedia.org/wiki/Year#Summary
/******************************************************************************/
namespace EE{
/******************************************************************************/
Str SteamWorks::StoreLink(C Str &app_id) {return S+"https://store.steampowered.com/app/"+app_id;}
/******************************************************************************/
enum OPERATION_TYPE
{
   PURCHASE          ,
   FINALIZE          ,
   QUERY_PURCHASE    ,
   QUERY_SUBSCRIPTION,
};
/******************************************************************************/
#if SUPPORT_STEAM
static struct SteamCallbacks // !! do not remove this !!
{
   STEAM_CALLBACK(SteamCallbacks, MicroTxnAuthorizationResponse, MicroTxnAuthorizationResponse_t, m_MicroTxnAuthorizationResponse);
   STEAM_CALLBACK(SteamCallbacks, PersonaStateChange           ,            PersonaStateChange_t, m_PersonaStateChange           );
   STEAM_CALLBACK(SteamCallbacks, AvatarImageLoaded            ,             AvatarImageLoaded_t, m_AvatarImageLoaded            );

   SteamCallbacks() : // this will register the callbacks using Steam API, using callbacks requires 'SteamUpdate' to be called
      m_MicroTxnAuthorizationResponse(this, &SteamCallbacks::MicroTxnAuthorizationResponse),
      m_PersonaStateChange           (this, &SteamCallbacks::PersonaStateChange           ),
      m_AvatarImageLoaded            (this, &SteamCallbacks::AvatarImageLoaded            )
   {}
}SC;

void SteamCallbacks::MicroTxnAuthorizationResponse(MicroTxnAuthorizationResponse_t *response)
{
   if(response && response->m_unAppID==Steam.appID())
   {
      if(response->m_bAuthorized)
      {
         Memt<HTTPParam> p;
         p.New().set("key"    , Steam._web_api_key          , HTTP_POST);
         p.New().set("appid"  ,        response->m_unAppID  , HTTP_POST);
         p.New().set("orderid", (ULong)response->m_ulOrderID, HTTP_POST);

         SteamWorks::Operation &op=Steam._operations.New();
         op.type    =FINALIZE;
         op.order_id=response->m_ulOrderID;
         op.create(STEAM_API "/FinalizeTxn/V0001/", p);
      }else
      if(auto callback=Steam.order_callback)callback(SteamWorks::ORDER_USER_CANCELED, response->m_ulOrderID, S, null, null);
   }
}
void SteamCallbacks::PersonaStateChange(PersonaStateChange_t *change)
{
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_Name==k_EPersonaChangeName);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_Status==k_EPersonaChangeStatus);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_ComeOnline==k_EPersonaChangeComeOnline);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_GoneOffline==k_EPersonaChangeGoneOffline);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_GamePlayed==k_EPersonaChangeGamePlayed);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_GameServer==k_EPersonaChangeGameServer);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_Avatar==k_EPersonaChangeAvatar);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_JoinedSource==k_EPersonaChangeJoinedSource);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_LeftSource==k_EPersonaChangeLeftSource);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_RelationshipChanged==k_EPersonaChangeRelationshipChanged);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_NameFirstSet==k_EPersonaChangeNameFirstSet);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_FacebookInfo==k_EPersonaChangeFacebookInfo);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_Nickname==k_EPersonaChangeNickname);
   ASSERT(SteamWorks::FRIEND_STATE_CHANGE_SteamLevel==k_EPersonaChangeSteamLevel);
   if(change)if(auto callback=Steam.friend_state_changed)callback(change->m_ulSteamID, change->m_nChangeFlags);
}
void SteamCallbacks::AvatarImageLoaded(AvatarImageLoaded_t *avatar) // called when 'GetLargeFriendAvatar' was requested but not yet available, simply notify user with callback that new avatar is available for a user
{
   if(avatar)if(auto callback=Steam.friend_state_changed)callback(avatar->m_steamID.ConvertToUint64(), SteamWorks::FRIEND_STATE_CHANGE_Avatar);
}
#endif
SteamWorks Steam;
/******************************************************************************/
static void  SteamUpdate  () {Steam.update();}
static ULong SteamUnixTime()
{
#if SUPPORT_STEAM
   if(ISteamUtils *i=SteamUtils())return i->GetServerRealTime();
#endif
   return 0;
}
static Bool SetDateFromYYYYMMDD(DateTime &dt, C Str &text)
{
   dt.zero();
   if(text.length()==8)
   {
      REPA(text)if(!FlagTest(CharFlag(text[i]), CHARF_DIG))goto invalid;
      dt.year =CharInt(text[0])*1000
              +CharInt(text[1])* 100
              +CharInt(text[2])*  10
              +CharInt(text[3])*   1;
      dt.month=CharInt(text[4])*  10
              +CharInt(text[5])*   1;
      dt.day  =CharInt(text[6])*  10
              +CharInt(text[7])*   1;
      return true;
   }
invalid:
   return false;
}
/******************************************************************************/
SteamWorks::Purchase::Purchase()
{
   finalized=false;
   user_id=0;
   item_id=cost_in_cents=0;
   date.zero();
}
SteamWorks::Subscription::Subscription()
{
   active=false;
   item_id=cost_in_cents=frequency=0;
   period=DAY;
   created.zero();
   last_payment.zero();
   next_payment.zero();
}
#if 0 // following function was used to check that MONTH formula will work on every possible date
void Test()
{
   DateTime last_payment; last_payment.zero();
   last_payment.day=1;
   last_payment.month=1;
   REP(365*400) // include leap-years
   {
      for(Int frequency=1; frequency<=12; frequency++)
      {
         DateTime cur_time=last_payment; REP(frequency)cur_time.incMonth();
         ULong unix_time=cur_time.seconds();
         ULong expiry=last_payment.seconds();
         switch(2)
         {
            case 0: expiry+=     ( frequency*             1+1        )*86400; break; // 86400=number of seconds in a day (60*60*24), +1 for 1 day tolerance
            case 1: expiry+=     ( frequency*            14+1        )*86400; break; // 86400=number of seconds in a day (60*60*24), +1 for 1 day tolerance
            case 2: expiry+=Trunc((frequency*DAYS_PER_MONTH+2.21f)*24)* 3600; break; //  3600=number of seconds in hour  (60*60   ), +1 for 1 day tolerance
            case 3: expiry+=     ( frequency*           365+1        )*86400; break; // 86400=number of seconds in a day (60*60*24), +1 for 1 day tolerance
         }
         if(!(unix_time<=expiry))
         {
            Flt a=unix_time/86400.0f,
                b=expiry   /86400.0f;
            int z=0; // FAIL
         }
      }
      last_payment.incDay();
   }
   int z=0; // OK
}
#endif
Bool SteamWorks::Subscription::valid()C
{
   if(last_payment.valid())
   {
      ULong unix_time=SteamUnixTime(); if(!unix_time)return false; // we need to know Steam time
      ULong expiry=last_payment.seconds1970();
      switch(period)
      {
         case DAY  : expiry+=     ( frequency*             1+1        )*86400; break; // 86400=number of seconds in a  day  (60*60*24), +1 for 1 day tolerance
         case WEEK : expiry+=     ( frequency*            14+1        )*86400; break; // 86400=number of seconds in a  day  (60*60*24), +1 for 1 day tolerance
         case MONTH: expiry+=Trunc((frequency*DAYS_PER_MONTH+2.21f)*24)* 3600; break; //  3600=number of seconds in an hour (60*60   ),          day tolerance was calculated using 'Test' function above
         case YEAR : expiry+=     ( frequency*           365+1        )*86400; break; // 86400=number of seconds in a  day  (60*60*24), +1 for 1 day tolerance
      }
      return unix_time<=expiry;
   }
   return false;
}
/******************************************************************************/
SteamWorks:: SteamWorks() {_initialized=false; init();}
SteamWorks::~SteamWorks() {                    shut();}
/******************************************************************************/
SteamWorks& SteamWorks::webApiKey(C Str8 &web_api_key) {T._web_api_key=web_api_key; return T;}

#if SUPPORT_STEAM
extern void (*SteamSetTime)();
static void  _SteamSetTime() // this is called at least once
{
   if(ISteamUtils *i=SteamUtils())
   {
      Steam._start_time_s=i->GetSecondsSinceAppActive(); // 'GetSecondsSinceAppActive' is since Steam Client was started and not this application
   }
   if(App._callbacks.initialized())App._callbacks.include(SteamUpdate); // include only if initialized, as this may be called before 'App' constructor and it would crash
}
#endif

Bool SteamWorks::init()
{
#if SUPPORT_STEAM
   if(!_initialized)
   {
     _initialized=SteamAPI_Init();
      // we can't add 'SteamUpdate' to the 'App._callbacks' here because this may be called in the constructor and 'App._callbacks' may not be initialized yet, instead this will be done in '_SteamSetTime' which is called here and during Time creation where 'App._callbacks' will be initialized
      SteamSetTime=_SteamSetTime; // set function pointer, so when 'Time' gets initialized, it will call this function too
     _SteamSetTime(); // call anyway, in case 'Time' was already initialized
   }
#endif
   return _initialized;
}
void SteamWorks::shut()
{
#if SUPPORT_STEAM
   if(_initialized)
   {
      SteamAPI_Shutdown();
     _initialized=false;
   }
#endif
}
/******************************************************************************/
// GET
/******************************************************************************/
UInt SteamWorks::appID()C
{
#if SUPPORT_STEAM
   if(ISteamUtils *i=SteamUtils())return i->GetAppID();
#endif
   return 0;
}
ULong SteamWorks::userID()C
{
#if SUPPORT_STEAM
   if(ISteamUser *i=SteamUser())return i->GetSteamID().ConvertToUint64();
#endif
   return 0;
}
Str SteamWorks::userName()C
{
#if SUPPORT_STEAM
   if(ISteamFriends *i=SteamFriends())return FromUTF8(i->GetPersonaName());
#endif
   return S;
}
#if SUPPORT_STEAM
static SteamWorks::USER_STATUS SteamStatus(EPersonaState state)
{
   ASSERT(k_EPersonaStateMax==7);
   switch(state)
   {
      case k_EPersonaStateOffline: return SteamWorks::STATUS_OFFLINE;

      case k_EPersonaStateLookingToTrade:
      case k_EPersonaStateLookingToPlay :
      case k_EPersonaStateOnline        : return SteamWorks::STATUS_ONLINE;

      case k_EPersonaStateBusy  :
      case k_EPersonaStateAway  :
      case k_EPersonaStateSnooze: return SteamWorks::STATUS_AWAY;
   }
   return SteamWorks::STATUS_UNKNOWN;
}
#endif
SteamWorks::USER_STATUS SteamWorks::userStatus()C
{
#if SUPPORT_STEAM
   if(ISteamFriends *i=SteamFriends())return SteamStatus(i->GetPersonaState());
#endif
   return STATUS_UNKNOWN;
}
Bool SteamWorks::userAvatar(Image &image)C {return userAvatar(userID(), image);}
CChar8* SteamWorks::appLanguage()C
{
#if SUPPORT_STEAM
   if(ISteamApps *i=SteamApps())return i->GetCurrentGameLanguage();
#endif
   return null;
}
CChar8* SteamWorks::country()C
{
#if SUPPORT_STEAM
   if(ISteamUtils *i=SteamUtils())return i->GetIPCountry();
#endif
   return null;
}
DateTime SteamWorks::date()C
{
   DateTime dt;
#if SUPPORT_STEAM
   if(ULong unix_time=SteamUnixTime())dt.from1970s(unix_time);else
#endif
      dt.zero();
   return dt;
}
UInt SteamWorks::curTimeS()C
{
#if SUPPORT_STEAM
   if(ISteamUtils *i=SteamUtils())return i->GetSecondsSinceAppActive()-_start_time_s;
#endif
   return 0;
}
Bool SteamWorks::overlayAvailable()C
{
#if SUPPORT_STEAM
   if(ISteamUtils *i=SteamUtils())return i->IsOverlayEnabled();
#endif
   return false;
}
Bool SteamWorks::overlayAvailableMsgBox()C
{
#if SUPPORT_STEAM
   if(overlayAvailable())return true;
   Gui.msgBox("Enable Steam Game Overlay", "Making purchases requires \"Steam Game Overlay\" to be enabled.\nPlease go to \"Steam Client Settings \\ In-Game\", click on the \"Enable the Steam Overlay while in-game\" and restart this app.");
#endif
   return false;
}
Bool SteamWorks::drmExit()C
{
#if SUPPORT_STEAM
   UInt app_id=appID();
   if( !app_id || SteamAPI_RestartAppIfNecessary(app_id)){App.flag|=APP_EXIT_IMMEDIATELY; return true;}
#endif
   return false;
}
#if 0
SteamWorks::RESULT SteamWorks::getUserInfo()
{
   if(!init()           )return STEAM_NOT_INITIALIZED;
   if(!_web_api_key.is())return STEAM_NOT_SETUP;
   if(!userID()         )return USER_ID_UNAVAILABLE;

   Memt<HTTPParam> p;
   p.New().set("steamid", Steam.userID());
   d.create(STEAM_API "/GetUserInfo/V0001/", p);
}
#endif
/******************************************************************************/
// FRIENDS
/******************************************************************************/
Bool SteamWorks::getFriends(MemPtr<ULong> friend_ids)C
{
#if SUPPORT_STEAM
   if(ISteamFriends *f=SteamFriends())
   {
      UInt flags=k_EFriendFlagImmediate;
      friend_ids.setNum(f->GetFriendCount(flags)); REPAO(friend_ids)=f->GetFriendByIndex(i, flags).ConvertToUint64();
      return true;
   }
#endif
   friend_ids.clear(); return false;
}
Str SteamWorks::userName(ULong user_id)C
{
#if SUPPORT_STEAM
   if(ISteamFriends *i=SteamFriends())return FromUTF8(i->GetFriendPersonaName((uint64)user_id));
#endif
   return S;
}
SteamWorks::USER_STATUS SteamWorks::userStatus(ULong user_id)C
{
#if SUPPORT_STEAM
   if(ISteamFriends *i=SteamFriends())return SteamStatus(i->GetFriendPersonaState((uint64)user_id));
#endif
   return STATUS_UNKNOWN;
}
Bool SteamWorks::userAvatar(ULong user_id, Image &image)C
{
#if SUPPORT_STEAM
   if(ISteamFriends *i=SteamFriends())
   {
      Int image_id=i->GetLargeFriendAvatar((uint64)user_id); // request large avatar first, if not available then it will return -1 and 'AvatarImageLoaded' will be called where we notify of new avatar available
      if( image_id<0)image_id=i->GetMediumFriendAvatar((uint64)user_id); // if not yet loaded, then get medium size, this should always be available
      if(!image_id){image.del(); return true;} // according to steam headers, 0 means no image set
      if(ISteamUtils *i=SteamUtils())
      {
         UInt width=0, height=0;
         if(i->GetImageSize(image_id, &width, &height))
         if(image.createSoftTry(width, height, 1, IMAGE_R8G8B8A8))
         if(i->GetImageRGBA(image_id, image.data(), image.memUsage()))
            return true;
      }
   }
#endif
   image.del(); return false;
}
/******************************************************************************/
// ORDER
/******************************************************************************/
SteamWorks::RESULT SteamWorks::purchase(ULong order_id, Int item_id, C Str &item_name, Int cost_in_cents, C Str8 &currency)
{
   if(!init()           )return STEAM_NOT_INITIALIZED;
   if(!_web_api_key.is())return STEAM_NOT_SETUP;
   if(! appID()         )return  APP_ID_UNAVAILABLE;
   if(!userID()         )return USER_ID_UNAVAILABLE;

   Memt<HTTPParam> p;
   p.New().set("key", _web_api_key, HTTP_POST);
   p.New().set("steamid", userID(), HTTP_POST);
   p.New().set("appid"  ,  appID(), HTTP_POST);
   p.New().set("orderid", order_id, HTTP_POST);
   p.New().set("itemcount", 1, HTTP_POST);
   p.New().set("language", "EN", HTTP_POST);
   p.New().set("currency", currency, HTTP_POST);
   p.New().set("itemid[0]", item_id, HTTP_POST);
   p.New().set("qty[0]", 1, HTTP_POST);
   p.New().set("amount[0]", cost_in_cents, HTTP_POST);
   p.New().set("description[0]", item_name, HTTP_POST);

   SteamWorks::Operation &op=Steam._operations.New();
   op.type    =PURCHASE;
   op.order_id=order_id;
   op.create(STEAM_API "/InitTxn/V0002/", p);
   return WAITING;
}
SteamWorks::RESULT SteamWorks::subscribe(ULong order_id, Int item_id, C Str &item_name, Int cost_in_cents, PERIOD period, Int frequency, C Str8 &currency)
{
   if(!init()           )return STEAM_NOT_INITIALIZED;
   if(!_web_api_key.is())return STEAM_NOT_SETUP;
   if(! appID()         )return  APP_ID_UNAVAILABLE;
   if(!userID()         )return USER_ID_UNAVAILABLE;
   ULong unix_time=SteamUnixTime(); if(!unix_time)return STEAM_NOT_INITIALIZED; // use date from Steam in case the OS has an incorrect date set

   Memt<HTTPParam> p;
   p.New().set("key", _web_api_key, HTTP_POST);
   p.New().set("steamid", userID(), HTTP_POST);
   p.New().set("appid"  ,  appID(), HTTP_POST);
   p.New().set("orderid", order_id, HTTP_POST);
   p.New().set("itemcount", 1, HTTP_POST);
   p.New().set("language", "EN", HTTP_POST);
   p.New().set("currency", currency, HTTP_POST);
   p.New().set("itemid[0]", item_id, HTTP_POST);
   p.New().set("qty[0]", 1, HTTP_POST);
   p.New().set("amount[0]", cost_in_cents, HTTP_POST);
   p.New().set("description[0]", item_name, HTTP_POST);
   p.New().set("billingtype[0]", "steam", HTTP_POST);
   p.New().set("frequency[0]", frequency, HTTP_POST);
   p.New().set("recurringamt[0]", cost_in_cents, HTTP_POST);
   switch(period)
   {
      case DAY  : p.New().set("period[0]", "day"  , HTTP_POST); unix_time+=frequency*(86400    ); break; // 86400=number of seconds in a day (60*60*24)
      case WEEK : p.New().set("period[0]", "week" , HTTP_POST); unix_time+=frequency*(86400* 14); break; // 86400=number of seconds in a day (60*60*24)
      case MONTH: p.New().set("period[0]", "month", HTTP_POST); unix_time+=frequency*(86400* 30); break; // 86400=number of seconds in a day (60*60*24), use 30 as avg number of days in a month
      case YEAR : p.New().set("period[0]", "year" , HTTP_POST); unix_time+=frequency*(86400*365); break; // 86400=number of seconds in a day (60*60*24)
      default   : return ORDER_REQUEST_FAILED;
   }
   DateTime date; date.from1970s(unix_time);
   p.New().set("startdate[0]", TextInt(date.year, 4)+TextInt(date.month, 2)+TextInt(date.day, 2), HTTP_POST); // format is YYYYMMDD

   SteamWorks::Operation &op=Steam._operations.New();
   op.type    =PURCHASE;
   op.order_id=order_id;
   op.create(STEAM_API "/InitTxn/V0002/", p);
   return WAITING;
}
SteamWorks::RESULT SteamWorks::purchaseState(ULong order_id)
{
   if(!init()           )return STEAM_NOT_INITIALIZED;
   if(!_web_api_key.is())return STEAM_NOT_SETUP;
   if(! appID()         )return APP_ID_UNAVAILABLE;

   Memt<HTTPParam> p;
   p.New().set("key"    , _web_api_key);
   p.New().set("appid"  ,  appID()    );
   p.New().set("orderid",  order_id   );

   SteamWorks::Operation &op=Steam._operations.New();
   op.type    =QUERY_PURCHASE;
   op.order_id=order_id;
   op.create(STEAM_API "/QueryTxn/V0001/", p);
   return WAITING;
}
SteamWorks::RESULT SteamWorks::subscriptionState()
{
   if(!init()           )return STEAM_NOT_INITIALIZED;
   if(!_web_api_key.is())return STEAM_NOT_SETUP;
   if(! appID()         )return  APP_ID_UNAVAILABLE;
   if(!userID()         )return USER_ID_UNAVAILABLE;

   Memt<HTTPParam> p;
   p.New().set("key"    , _web_api_key);
   p.New().set("steamid",  userID()   );
   p.New().set("appid"  ,   appID()   );

   SteamWorks::Operation &op=Steam._operations.New();
   op.type    =QUERY_SUBSCRIPTION;
   op.order_id=0;
   op.create(STEAM_API "/GetUserAgreementInfo/V0001/", p);
   return WAITING;
}
void SteamWorks::update()
{
   REPA(_operations)
   {
      Operation &op=_operations[i]; if(op.state()==DWNL_ERROR || op.state()==DWNL_DONE) // have to process errors too, to report them into callback, try to read downloaded data from errors too, as they may contain some information
      {
         RESULT res;
         switch(op.type)
         {
            case PURCHASE          : res=ORDER_REQUEST_FAILED      ; break;
            case FINALIZE          : res=ORDER_FINALIZE_FAILED     ; break;
            case QUERY_PURCHASE    : res=PURCHASE_STATUS_FAILED    ; break;
            case QUERY_SUBSCRIPTION: res=SUBSCRIPTION_STATUS_FAILED; break;
            default                : res=STEAM_NOT_INITIALIZED     ; break;
         }
         Str          error_message;
         Purchase     purchase    , *    purchase_ptr=null;
         Subscription subscription, *subscription_ptr=null;
         TextData     data; FileText f; data.loadJSON(f.readMem(op.data(), op.done()));
      #if DEBUG && 0
         data.save(FFirst("d:/", "txt"));
      #endif
         if(data.nodes.elms()==1)if(TextNode *response=data.nodes[0].findNode("response"))
         {
            TextNode *result=response->findNode("result");
            if(result && result->asText()=="OK")
            {
               TextNode *params=response->findNode("params");
               if(op.type==QUERY_SUBSCRIPTION) // subscription status
               {
                  res=SUBSCRIPTION_STATUS_OK;
                  if(params)if(TextNode *agreements=params->findNode("agreements"))if(agreements->nodes.elms()==1) // if this is null or it has no elements, then there are no subscriptions
                  {
                     subscription_ptr=&subscription;
                     TextNode &agreement=agreements->nodes[0];
                     if(TextNode *p=agreement.findNode("status"      ))subscription.active       =(p->asText()=="Active");
                     if(TextNode *p=agreement.findNode("itemid"      ))subscription.item_id      = p->asInt ();
                     if(TextNode *p=agreement.findNode("frequency"   ))subscription.frequency    = p->asInt ();
                     if(TextNode *p=agreement.findNode("recurringamt"))subscription.cost_in_cents= p->asInt ();
                     if(TextNode *p=agreement.findNode("currency"    ))subscription.currency     = p->asText();
                     if(TextNode *p=agreement.findNode("timecreated" ))SetDateFromYYYYMMDD(subscription.created     , p->asText());
                     if(TextNode *p=agreement.findNode("nextpayment" ))SetDateFromYYYYMMDD(subscription.next_payment, p->asText());
                     if(TextNode *p=agreement.findNode("lastpayment" ))SetDateFromYYYYMMDD(subscription.last_payment, p->asText()); if(!subscription.last_payment.valid())subscription.last_payment=subscription.created; // "lastpayment" value can be "NIL", so in case that happens or it wasn't specified at all, then use the 'subscription.created'
                     if(TextNode *p=agreement.findNode("period"      ))
                     {
                        if(p->asText()=="day"  )subscription.period=DAY  ;else
                        if(p->asText()=="week" )subscription.period=WEEK ;else
                        if(p->asText()=="month")subscription.period=MONTH;else
                        if(p->asText()=="year" )subscription.period=YEAR ;
                     }
                  }
               }else
               if(params)if(TextNode *order_id=params->findNode("orderid"))if(order_id->asULong()==op.order_id)
               {
                  switch(op.type)
                  {
                     case PURCHASE      : res=ORDER_REQUEST_OK ; break;
                     case FINALIZE      : res=ORDER_FINALIZE_OK; break;
                     case QUERY_PURCHASE:
                     {
                        res=PURCHASE_STATUS_OK;
                        purchase_ptr=&purchase;
                        if(TextNode *p=params->findNode("status"  ))purchase.finalized=(p->asText ()=="Succeeded");
                        if(TextNode *p=params->findNode("steamid" ))purchase.user_id  = p->asULong();
                        if(TextNode *p=params->findNode("currency"))purchase.currency = p->asText ();
                        if(TextNode *p=params->findNode("country" ))purchase.country  = p->asText ();
                        if(TextNode *p=params->findNode("time"    ))purchase.date.fromText(p->value.replace('T', ' ').replace('Z', '\0'));
                        if(TextNode *items=params->findNode("items"))if(items->nodes.elms()==1)
                        {
                           TextNode &item=items->nodes[0];
                           if(TextNode *itemid=item.findNode("itemid"))purchase.item_id      =itemid->asInt();
                           if(TextNode *amount=item.findNode("amount"))purchase.cost_in_cents=amount->asInt();
                        }
                     }break;
                  }
               }
            }else
            if(TextNode *error=response->findNode("error"))
            {
               if(TextNode *errordesc=error->findNode("errordesc"))Swap(error_message, errordesc->value);
             /*if(TextNode *errorcode=error->findNode("errorcode"))switch(errorcode->asInt())
               {
                  case 
               }*/
            }
            goto finish;
         }
         f.rewind(); f.getAll(error_message);
      finish:
         if(auto callback=order_callback)callback(res, op.order_id, error_message, purchase_ptr, subscription_ptr);
        _operations.removeValid(i);
      }
   }
#if SUPPORT_STEAM
   SteamAPI_RunCallbacks();
#endif
   App._callbacks.add(SteamUpdate);
}
/******************************************************************************/
// CLOUD SAVES
/******************************************************************************/
Long SteamWorks::cloudAvailableSize()C
{
#if SUPPORT_STEAM
   if(ISteamRemoteStorage *i=SteamRemoteStorage()){uint64 total, available; if(i->GetQuota(&total, &available))return available;}
#endif
   return 0;
}
Long SteamWorks::cloudTotalSize()C
{
#if SUPPORT_STEAM
   if(ISteamRemoteStorage *i=SteamRemoteStorage()){uint64 total, available; if(i->GetQuota(&total, &available))return total;}
#endif
   return 0;
}

Bool SteamWorks::cloudDel(C Str &file_name)
{
#if SUPPORT_STEAM
   if(ISteamRemoteStorage *i=SteamRemoteStorage())return i->FileDelete(UTF8(file_name));
#endif
   return false;
}
Bool SteamWorks::cloudExists(C Str &file_name)
{
#if SUPPORT_STEAM
   if(ISteamRemoteStorage *i=SteamRemoteStorage())return i->FileExists(UTF8(file_name));
#endif
   return false;
}
Long SteamWorks::cloudSize(C Str &file_name)
{
#if SUPPORT_STEAM
   if(ISteamRemoteStorage *i=SteamRemoteStorage())return i->GetFileSize(UTF8(file_name));
#endif
   return 0;
}
DateTime SteamWorks::cloudTimeUTC(C Str &file_name)
{
   DateTime dt;
#if SUPPORT_STEAM
   if(ISteamRemoteStorage *i=SteamRemoteStorage())
      if(int64 timestamp=i->GetFileTimestamp(UTF8(file_name)))
         return dt.from1970s(timestamp);
#endif
   return dt.zero();
}

Bool SteamWorks::cloudSave(C Str &file_name, File &f, Cipher *cipher)
{
#if SUPPORT_STEAM
   if(file_name.is())if(ISteamRemoteStorage *i=SteamRemoteStorage())
   {
      Memt<Byte> data; data.setNum(f.left()); if(f.get(data.data(), data.elms()))
      {
         if(cipher)cipher->encrypt(data.data(), data.data(), data.elms(), 0);
         return i->FileWrite(UTF8(file_name), data.data(), data.elms());
      }
   }
#endif
   return false;
}
Bool SteamWorks::cloudLoad(C Str &file_name, File &f, Bool memory, Cipher *cipher)
{
#if SUPPORT_STEAM
   if(file_name.is())if(ISteamRemoteStorage *i=SteamRemoteStorage())
   {
      Str8 name=UTF8(file_name);
      Long size=i->GetFileSize(name);
      if(size>0 || size==0 && i->FileExists(name))
      {
         if(size>0)
         {
            Memt<Byte> data; data.setNum(size);
         #if CLOUD_WORKAROUND
            REPD(attempt, 1000)
         #endif
            {
               Int read=i->FileRead(name, data.data(), data.elms());
               if( read==data.elms())
               {
                  if(cipher)cipher->decrypt(data.data(), data.data(), data.elms(), 0);
                  if(memory)f.writeMemFixed(size);
                  if(f.put(data.data(), data.elms()))return true;
               #if CLOUD_WORKAROUND
                  break;
               #endif
               }
            #if CLOUD_WORKAROUND
             //LogN(S+"failed:"+attempt+" "+read+'/'+size);
               Time.wait(1);
            #endif
            }
         }else // "size==0"
         {
            f.writeMemFixed(0);
            return true;
         }
      }
   }
#endif
   if(memory)f.close(); return false;
}

Bool SteamWorks::cloudSave(C Str &file_name, CPtr data, Int size)
{
#if SUPPORT_STEAM
   if(file_name.is())if(ISteamRemoteStorage *i=SteamRemoteStorage())return i->FileWrite(UTF8(file_name), data, size);
#endif
   return false;
}
Bool SteamWorks::cloudLoad(C Str &file_name, Ptr data, Int size)
{
#if SUPPORT_STEAM
   if(file_name.is())if(ISteamRemoteStorage *i=SteamRemoteStorage())return i->FileRead(UTF8(file_name), data, size)==size;
#endif
   return false;
}

Int SteamWorks::cloudFiles()C
{
#if SUPPORT_STEAM
   if(ISteamRemoteStorage *i=SteamRemoteStorage())return i->GetFileCount();
#endif
   return 0;
}
Bool SteamWorks::cloudFile(Int file_index, Str &name, Long &size)C
{
#if SUPPORT_STEAM
   if(file_index>=0)if(ISteamRemoteStorage *i=SteamRemoteStorage())
   {
      int32 file_size; CChar8 *file_name=i->GetFileNameAndSize(file_index, &file_size);
      if(Is(file_name))
      {
         name=FromUTF8(file_name);
         size=file_size;
         return true;
      }
   }
#endif
   name.clear(); size=0; return false;
}
/******************************************************************************/
}
/******************************************************************************/
