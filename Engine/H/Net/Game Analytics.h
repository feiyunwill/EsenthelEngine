/******************************************************************************

   This is a wrapper for "Game Analytics" - https://gameanalytics.com/

   For documentation and usage please refer to the official "Game Analytics" pages:
      Docs    - https://gameanalytics.com/docs
      iOS     - https://gameanalytics.com/docs/item/ios-sdk
      Android - https://gameanalytics.com/docs/item/android-sdk

/******************************************************************************/
#define   GameAnalytics _GameAnalytics
namespace GameAnalytics
{
   enum GAResourceFlowType
   {
      GAResourceFlowTypeSource=1,
      GAResourceFlowTypeSink  =2,
   };
   enum GAProgressionStatus
   {
      GAProgressionStatusStart   =1,
      GAProgressionStatusComplete=2,
      GAProgressionStatusFail    =3,
   };
   enum GAErrorSeverity
   {
      GAErrorSeverityDebug   =1,
      GAErrorSeverityInfo    =2,
      GAErrorSeverityWarning =3,
      GAErrorSeverityError   =4,
      GAErrorSeverityCritical=5,
   };

   void configureAvailableCustomDimensions01(C MemPtr<Str> &customDimensions);
   void configureAvailableCustomDimensions02(C MemPtr<Str> &customDimensions);
   void configureAvailableCustomDimensions03(C MemPtr<Str> &customDimensions);
   void configureAvailableResourceCurrencies(C MemPtr<Str> &resourceCurrencies);
   void configureAvailableResourceItemTypes (C MemPtr<Str> &resourceItemTypes);

   void configureBuild(C Str &build);
   void configureUserId(C Str &userId);
   void configureEngineVersion(C Str &engineVersion=ENGINE_BUILD);

   void initialize(C Str &gameKey, C Str &gameSecret);

   void addBusinessEventWithCurrency(C Str &currency, Int amount, C Str &itemType, C Str &itemId, C Str &cartType, C Str &receipt);
   void addBusinessEventWithCurrency(C Str &currency, Int amount, C Str &itemType, C Str &itemId, C Str &cartType, Bool autoFetchReceipt);
   void addResourceEventWithFlowType(GAResourceFlowType flowType, C Str &currency, Dbl amount, C Str &itemType, C Str &itemId);
   void addProgressionEventWithProgressionStatus(GAProgressionStatus progressionStatus, C Str &progression01, C Str &progression02, C Str &progression03);
   void addProgressionEventWithProgressionStatus(GAProgressionStatus progressionStatus, C Str &progression01, C Str &progression02, C Str &progression03, Int score);
   void addDesignEventWithEventId(C Str &eventId);
   void addDesignEventWithEventId(C Str &eventId, Dbl value);
   void addErrorEventWithSeverity(GAErrorSeverity severity, C Str &message);

   Str getCommandCenterValueAsString(C Str &key);
   Str getCommandCenterValueAsString(C Str &key, C Str &defaultValue);
   Str getCommandCenterConfigurations();
#if EE_PRIVATE
 //void setCommandCenterDelegate(id newDelegate);
#endif
   Bool isCommandCenterReady();

   void setEnabledInfoLog(Bool flag);
   void setEnabledVerboseLog(Bool flag);

   void setEnabledManualSessionHandling(Bool flag);
   void startSession();
   void endSession();

   void setCustomDimension01(C Str &dimension01);
   void setCustomDimension02(C Str &dimension02);
   void setCustomDimension03(C Str &dimension03);
   void setFacebookId(C Str &facebookId);
   void setGender(C Str &gender);
   void setBirthYear(Int birthYear);
}
/******************************************************************************/
