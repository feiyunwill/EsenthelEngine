/******************************************************************************/
#include "stdafx.h"
#define SUPPORT_GAME_ANALYTICS 1
#undef  GameAnalytics

#include "../../../ThirdPartyLibs/begin.h"
#if IOS && SUPPORT_GAME_ANALYTICS
   #include "../../../ThirdPartyLibs/GameAnalytics/iOS/GameAnalytics.h"
   ASSERT((Int)::GAResourceFlowTypeSource==EE::_GameAnalytics::GAResourceFlowTypeSource && (Int)::GAResourceFlowTypeSink==EE::_GameAnalytics::GAResourceFlowTypeSink
       && (Int)::GAProgressionStatusStart==EE::_GameAnalytics::GAProgressionStatusStart && (Int)::GAProgressionStatusComplete==EE::_GameAnalytics::GAProgressionStatusComplete && (Int)::GAProgressionStatusFail==EE::_GameAnalytics::GAProgressionStatusFail
       && (Int)::GAErrorSeverityDebug==EE::_GameAnalytics::GAErrorSeverityDebug && (Int)::GAErrorSeverityInfo==EE::_GameAnalytics::GAErrorSeverityInfo && (Int)::GAErrorSeverityWarning==EE::_GameAnalytics::GAErrorSeverityWarning && (Int)::GAErrorSeverityError==EE::_GameAnalytics::GAErrorSeverityError && (Int)::GAErrorSeverityCritical==EE::_GameAnalytics::GAErrorSeverityCritical);
#elif ANDROID && SUPPORT_GAME_ANALYTICS
   #include "../../../ThirdPartyLibs/GameAnalytics/Android/GameAnalyticsJNI.h"
#endif
#include "../../../ThirdPartyLibs/end.h"

namespace EE{namespace _GameAnalytics{
/******************************************************************************/
void configureAvailableCustomDimensions01(C MemPtr<Str> &customDimensions)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   if(NSMutableArray *array=[NSMutableArray arrayWithCapacity:customDimensions.elms()])
   {
      FREPA(customDimensions)[array addObject:NSStringAuto(customDimensions[i])];
      [GameAnalytics configureAvailableCustomDimensions01:array];
    //[array release]; NSMutableArray from 'arrayWithCapacity' can't be released
   }
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void configureAvailableCustomDimensions02(C MemPtr<Str> &customDimensions)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   if(NSMutableArray *array=[NSMutableArray arrayWithCapacity:customDimensions.elms()])
   {
      FREPA(customDimensions)[array addObject:NSStringAuto(customDimensions[i])];
      [GameAnalytics configureAvailableCustomDimensions02:array];
    //[array release]; NSMutableArray from 'arrayWithCapacity' can't be released
   }
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void configureAvailableCustomDimensions03(C MemPtr<Str> &customDimensions)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   if(NSMutableArray *array=[NSMutableArray arrayWithCapacity:customDimensions.elms()])
   {
      FREPA(customDimensions)[array addObject:NSStringAuto(customDimensions[i])];
      [GameAnalytics configureAvailableCustomDimensions03:array];
    //[array release]; NSMutableArray from 'arrayWithCapacity' can't be released
   }
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void configureAvailableResourceCurrencies(C MemPtr<Str> &resourceCurrencies)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   if(NSMutableArray *array=[NSMutableArray arrayWithCapacity:resourceCurrencies.elms()])
   {
      FREPA(resourceCurrencies)[array addObject:NSStringAuto(resourceCurrencies[i])];
      [GameAnalytics configureAvailableResourceCurrencies:array];
    //[array release]; NSMutableArray from 'arrayWithCapacity' can't be released
   }
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void configureAvailableResourceItemTypes(C MemPtr<Str> &resourceItemTypes)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   if(NSMutableArray *array=[NSMutableArray arrayWithCapacity:resourceItemTypes.elms()])
   {
      FREPA(resourceItemTypes)[array addObject:NSStringAuto(resourceItemTypes[i])];
      [GameAnalytics configureAvailableResourceItemTypes:array];
    //[array release]; NSMutableArray from 'arrayWithCapacity' can't be released
   }
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}

void configureBuild(C Str &build)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics configureBuild:NSStringAuto(build)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void configureUserId(C Str &userId)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics configureUserId:NSStringAuto(userId)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void configureEngineVersion(C Str &engineVersion)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics configureEngineVersion:NSStringAuto(engineVersion)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}

void initialize(C Str &gameKey, C Str &gameSecret)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics initializeWithGameKey:NSStringAuto(gameKey) gameSecret:NSStringAuto(gameSecret)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}

void addBusinessEventWithCurrency(C Str &currency, Int amount, C Str &itemType, C Str &itemId, C Str &cartType, C Str &receipt)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics addBusinessEventWithCurrency:NSStringAuto(currency) amount:amount itemType:NSStringAuto(itemType) itemId:NSStringAuto(itemId) cartType:NSStringAuto(cartType) receipt:NSStringAuto(receipt)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void addBusinessEventWithCurrency(C Str &currency, Int amount, C Str &itemType, C Str &itemId, C Str &cartType, Bool autoFetchReceipt)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics addBusinessEventWithCurrency:NSStringAuto(currency) amount:amount itemType:NSStringAuto(itemType) itemId:NSStringAuto(itemId) cartType:NSStringAuto(cartType) autoFetchReceipt:autoFetchReceipt];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void addResourceEventWithFlowType(GAResourceFlowType flowType, C Str &currency, Dbl amount, C Str &itemType, C Str &itemId)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics addResourceEventWithFlowType:(::GAResourceFlowType)flowType currency:NSStringAuto(currency) amount:[NSNumber numberWithDouble:amount] itemType:NSStringAuto(itemType) itemId:NSStringAuto(itemId)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void addProgressionEventWithProgressionStatus(GAProgressionStatus progressionStatus, C Str &progression01, C Str &progression02, C Str &progression03)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics addProgressionEventWithProgressionStatus:(::GAProgressionStatus)progressionStatus progression01:NSStringAuto(progression01) progression02:NSStringAuto(progression02) progression03:NSStringAuto(progression03)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void addProgressionEventWithProgressionStatus(GAProgressionStatus progressionStatus, C Str &progression01, C Str &progression02, C Str &progression03, Int score)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics addProgressionEventWithProgressionStatus:(::GAProgressionStatus)progressionStatus progression01:NSStringAuto(progression01) progression02:NSStringAuto(progression02) progression03:NSStringAuto(progression03) score:score];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void addDesignEventWithEventId(C Str &eventId)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics addDesignEventWithEventId:NSStringAuto(eventId)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void addDesignEventWithEventId(C Str &eventId, Dbl value)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics addDesignEventWithEventId:NSStringAuto(eventId) value:[NSNumber numberWithDouble:value]];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void addErrorEventWithSeverity(GAErrorSeverity severity, C Str &message)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics addErrorEventWithSeverity:(::GAErrorSeverity)severity message:NSStringAuto(message)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}

Str getCommandCenterValueAsString(C Str &key)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   return [GameAnalytics getCommandCenterValueAsString:NSStringAuto(key)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
   return S;
}
Str getCommandCenterValueAsString(C Str &key, C Str &defaultValue)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   return [GameAnalytics getCommandCenterValueAsString:NSStringAuto(key) defaultValue:NSStringAuto(defaultValue)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
   return S;
}
Str getCommandCenterConfigurations()
{
#if IOS && SUPPORT_GAME_ANALYTICS
   return [GameAnalytics getCommandCenterConfigurations];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
   return S;
}
/*void setCommandCenterDelegate(id newDelegate)
{
#if IOS && SUPPORT_GAME_ANALYTICS
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}*/
Bool isCommandCenterReady()
{
#if IOS && SUPPORT_GAME_ANALYTICS
   return [GameAnalytics isCommandCenterReady];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
   return false;
}

void setEnabledInfoLog(Bool flag)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics setEnabledInfoLog:flag];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void setEnabledVerboseLog(Bool flag)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics setEnabledVerboseLog:flag];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}

void setEnabledManualSessionHandling(Bool flag)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics setEnabledManualSessionHandling:flag];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void startSession()
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics startSession];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void endSession()
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics endSession];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}

void setCustomDimension01(C Str &dimension01)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics setCustomDimension01:NSStringAuto(dimension01)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void setCustomDimension02(C Str &dimension02)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics setCustomDimension02:NSStringAuto(dimension02)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void setCustomDimension03(C Str &dimension03)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics setCustomDimension03:NSStringAuto(dimension03)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void setFacebookId(C Str &facebookId)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics setFacebookId:NSStringAuto(facebookId)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void setGender(C Str &gender)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics setGender:NSStringAuto(gender)];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
void setBirthYear(Int birthYear)
{
#if IOS && SUPPORT_GAME_ANALYTICS
   [GameAnalytics setBirthYear:birthYear];
#elif ANDROID && SUPPORT_GAME_ANALYTICS
#endif
}
/******************************************************************************/
}}
/******************************************************************************/
