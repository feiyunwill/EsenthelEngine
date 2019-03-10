/******************************************************************************/
package EE_PACKAGE;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Application;
import android.app.NativeActivity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.app.TaskStackBuilder;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.DialogInterface.OnShowListener;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Configuration;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.graphics.BitmapFactory;
import android.location.Location;
import android.location.LocationListener;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.media.MediaRecorder.AudioSource;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.WifiLock;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.IBinder;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.StatFs;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.provider.Settings.Secure;
import android.text.Editable;
import android.text.InputType;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.EditorInfo;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.Toast;
import com.android.vending.billing.IInAppBillingService;
import com.chartboost.sdk.*;
import com.chartboost.sdk.Model.CBError.CBImpressionError;
import com.google.android.gms.ads.*;
import com.facebook.android.DialogError;
import com.facebook.android.Facebook;
import com.facebook.android.Facebook.DialogListener;
import com.facebook.android.FacebookError;
import com.facebook.HttpMethod;
import com.facebook.LoggingBehavior;
import com.facebook.model.GraphUser;
import com.facebook.model.GraphObject;
import com.facebook.Response;
import com.facebook.Request;
import com.facebook.Request.GraphUserListCallback;
import com.facebook.Session;
import com.facebook.SessionState;
//import com.facebook.Settings; can't import due to 'android.provider.Settings'
import com.facebook.UiLifecycleHelper;
import com.facebook.widget.FacebookDialog;
import com.facebook.widget.WebDialog;
import com.facebook.widget.WebDialog.OnCompleteListener;
import java.io.File;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.security.KeyFactory;
import java.security.MessageDigest;
import java.security.PublicKey;
import java.security.Signature;
import java.security.spec.X509EncodedKeySpec;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import org.json.JSONObject;
/******************************************************************************/
public class EsenthelActivity extends NativeActivity
{
   public static class EsenthelLocationListener implements LocationListener
   {
      public boolean gps;

      public EsenthelLocationListener(boolean gps) {this.gps=gps;}

      @Override public final void onLocationChanged (Location   location) {com.esenthel.Native.location(gps, location);}
      @Override public final void onProviderDisabled(String providerName) {}
      @Override public final void onProviderEnabled (String providerName) {}
      @Override public final void onStatusChanged   (String providerName, int status, Bundle extras) {}
   }
   public static class DetectForceKill extends Service // !! if changing name of this class then update AndroidManifest.xml !!
   {
      @Override public final IBinder onBind        (Intent intent) {return null;}
      @Override public final int     onStartCommand(Intent intent, int flags, int startId) {return START_NOT_STICKY;}
    //@Override public final void    onDestroy     () {super.onDestroy();}
      @Override public final void    onTaskRemoved (Intent rootIntent) {removeNotifications(); super.onTaskRemoved(rootIntent);} // this is called on force kill
   }

   // !! these must be equal to 'AWAKE_MODE' !!
   public static final int AWAKE_OFF   =0;
   public static final int AWAKE_SYSTEM=1;
   public static final int AWAKE_SCREEN=2;

   // !! these must be equal to 'AdMobClass.BANNER_TYPE' !!
   public static final int AD_BANNER          =0;
   public static final int AD_MEDIUM_RECTANGLE=1;
   public static final int AD_FULL_BANNER     =2;
   public static final int AD_LEADERBOARD     =3;
   public static final int AD_SMART_BANNER    =4;
   public static final int AD_INTERSTITIAL    =5;

   // !! these must be equal to 'AdMobClass.STATE' !!
   public static final int AD_NONE   =0;
   public static final int AD_LOADING=1;
   public static final int AD_DONE   =2;
   public static final int AD_ERROR  =3;

   // !! these must be equal to 'ChartboostClass.RESULT' !!
   public static final int INTERSTITIAL_LOADED   =0;
   public static final int INTERSTITIAL_LOAD_FAIL=1;
   public static final int INTERSTITIAL_DISPLAYED=2;
   public static final int INTERSTITIAL_CLOSED   =3;
   public static final int INTERSTITIAL_CLICKED  =4;
   public static final int REWARDED_VIDEO_LOADED   =5;
   public static final int REWARDED_VIDEO_LOAD_FAIL=6;
   public static final int REWARDED_VIDEO_DISPLAYED=7;
   public static final int REWARDED_VIDEO_CLOSED   =8;
   public static final int REWARDED_VIDEO_COMPLETED=9;
   public static final int REWARDED_VIDEO_CLICKED  =10;

   // !! these must be equal to 'PlatformStore.RESULT' !!
   public static final int RES_OK                 =-1; // this is not a 'PlatformStore.RESULT' but a temp value
   public static final int RES_PURCHASED          =0;
   public static final int RES_CONSUMED           =1;
   public static final int RES_REFUND             =2;
   public static final int RES_WAITING            =3;
   public static final int RES_USER_CANCELED      =4;
   public static final int RES_SERVICE_CANCELED   =5;
   public static final int RES_SERVICE_UNAVAILABLE=6;
   public static final int RES_ITEM_UNAVAILABLE   =7;
   public static final int RES_ALREADY_OWNED      =8;
   public static final int RES_NOT_OWNED          =9;
   public static final int RES_VERIFICATION_FAIL  =10;
   public static final int RES_UNKNOWN            =11;
   public static final int RES_REFRESHED_ITEMS    =12;
   public static final int RES_REFRESHED_PURCHASES=13;

   // !! these must be equal to 'Facebook.RESULT' !!
   public static final int POST_ERROR=0;
   public static final int POST_CANCEL=1;
   public static final int POST_SUCCESS=2;
   public static final int POST_NOT_LOGGED_IN=3;

   // Billing response codes
   public static final int BILLING_RESPONSE_RESULT_OK = 0;
   public static final int BILLING_RESPONSE_RESULT_USER_CANCELED = 1;
   public static final int BILLING_RESPONSE_RESULT_BILLING_UNAVAILABLE = 3;
   public static final int BILLING_RESPONSE_RESULT_ITEM_UNAVAILABLE = 4;
   public static final int BILLING_RESPONSE_RESULT_DEVELOPER_ERROR = 5;
   public static final int BILLING_RESPONSE_RESULT_ERROR = 6;
   public static final int BILLING_RESPONSE_RESULT_ITEM_ALREADY_OWNED = 7;
   public static final int BILLING_RESPONSE_RESULT_ITEM_NOT_OWNED = 8;

   // Item types
   public static final String ITEM_TYPE_INAPP="inapp";
   public static final String ITEM_TYPE_SUBS ="subs";

   // some fields on the getSkuDetails response bundle
   public static final String GET_SKU_DETAILS_ITEM_LIST="ITEM_ID_LIST";

   // Keys for the responses from InAppBillingService
   public static final String RESPONSE_CODE = "RESPONSE_CODE";
   public static final String RESPONSE_GET_SKU_DETAILS_LIST = "DETAILS_LIST";
   public static final String RESPONSE_BUY_INTENT = "BUY_INTENT";
   public static final String RESPONSE_INAPP_PURCHASE_DATA = "INAPP_PURCHASE_DATA";
   public static final String RESPONSE_INAPP_SIGNATURE = "INAPP_DATA_SIGNATURE";
   public static final String RESPONSE_INAPP_ITEM_LIST = "INAPP_PURCHASE_ITEM_LIST";
   public static final String RESPONSE_INAPP_PURCHASE_DATA_LIST = "INAPP_PURCHASE_DATA_LIST";
   public static final String RESPONSE_INAPP_SIGNATURE_LIST = "INAPP_DATA_SIGNATURE_LIST";
   public static final String INAPP_CONTINUATION_TOKEN = "INAPP_CONTINUATION_TOKEN";

   // signature verification
   private static final String KEY_FACTORY_ALGORITHM="RSA";
   private static final String   SIGNATURE_ALGORITHM="SHA1withRSA";

   public static final int REQUEST_CODE_IAB=0;

   // variables
   static EsenthelActivity     activity;
   static Application          application;
   static Context              context;
   static Intent               background_intent;
   static String               background_text;

   static Handler              handler;
   static IInAppBillingService iab_service;
   static ServiceConnection    iab_service_conn;
   static PublicKey            license_key;
   static AdRequest            ad_request;
          AdView               ad_view;
   static boolean              ad_view_loaded=false, // if ad_view is loaded
                               banner_visible=false, // if banner should be visible
                               inters_show=false; // if show interstitial once it's loaded
   static String               banner_id, // id of the banner       ad, null for none
                               inters_id; // id of the interstitial ad, null for none
   static int                  banner_type=AD_BANNER, // banner type
                               banner_x=0, banner_y=1; // banner position, -1..1
          InterstitialAd       interstitial;
          PopupWindow          popup_window;
          LinearLayout         popup_window_layout, ad_view_layout;
   static String               android_id;
          EditText             edit_text;
          TextWatcher          text_watcher;
   static WakeLock             wake_lock;
   static WifiLock             wifi_lock;

   static final void   log(String s) {Log.e("Esenthel", s);}
   static final boolean Is(String s) {return !TextUtils.isEmpty(s);}

   static
   {
      EE_LOAD_LIBRARIES
   }
   /******************************************************************************/
   // MAIN
   /******************************************************************************/
   @Override public final void onSaveInstanceState(Bundle savedInstanceState)
   {
      super.onSaveInstanceState(savedInstanceState);
      Session.saveSession(Session.getActiveSession(), savedInstanceState);
      if(ui_helper!=null)ui_helper.onSaveInstanceState(savedInstanceState);
   }
   @Override public final void onRestoreInstanceState(Bundle savedInstanceState)
   {
      super.onRestoreInstanceState(savedInstanceState);
      if(Is(inters_id))adCreate(inters_id, AD_INTERSTITIAL);
      if(Is(banner_id))adCreate(banner_id, banner_type    );
   }
   @Override public final void onNewIntent(Intent intent)
   {
      super.onNewIntent(intent);
      checkNotification(intent, true);
   }
   @Override public final void onBackPressed()
   {
      if(!Chartboost.onBackPressed()) // this will detect if there's an interstitial and close it, otherwise process below:
      {
         super.onBackPressed();
      }
   }
   @Override public final void onResume()
   {
                             super.onResume();
      if(ad_view  !=null)ad_view  .  resume();
      if(ui_helper!=null)ui_helper.onResume();
                        Chartboost.onResume(this);
   }
   @Override public final void onPause()
   {
      if(ad_view  !=null)ad_view  .  pause();
      if(ui_helper!=null)ui_helper.onPause();
                             super.onPause();
                        Chartboost.onPause(this);
   }
   @Override public final void onStart()
   {
           super.onStart();
      Chartboost.onStart(this);
      Session session=Session.getActiveSession(); if(session!=null)session.addCallback(fb_status_callback);
   }
   @Override public final void onStop()
   {
           super.onStop();
      Chartboost.onStop(this);
      Session session=Session.getActiveSession(); if(session!=null)session.removeCallback(fb_status_callback);
   }
   @Override public final void onDestroy()
   {
      bannerHide();
      adViewDel();
                             super.onDestroy();
      if(ui_helper!=null)ui_helper.onDestroy();
                        Chartboost.onDestroy(this);
      shutIAB();
      context=application; activity=null; // when activity becomes unavailable, then use application context because we always need one
   }
   @Override public final void onCreate(Bundle savedInstanceState)
   {
      application=getApplication(); context=activity=this; // use activity when available, as it's more powerful - https://stackoverflow.com/questions/4128589/difference-between-activity-context-and-application-context

      if(handler==null)handler=new Handler()
      {
         @Override public final void handleMessage(Message message)
         {
            super.handleMessage(message);  
            String  title=message.getData().getString ("title"),
                    text =message.getData().getString ("text" );
            boolean exit =message.getData().getBoolean("exit" );
            if(title!=null && text!=null)
            {
               AlertDialog dialog=new AlertDialog.Builder(context).setTitle(title).setMessage(text).setNeutralButton("OK", null).show();
               if(exit && dialog!=null)
                  dialog.setOnDismissListener(new DialogInterface.OnDismissListener()
                  {
                     @Override public final void onDismiss(final DialogInterface dialog) {com.esenthel.Native.closedError();} // can't call "activity.finish();" because it may suspend the app, while we need to force close it
                  });
            }
         }
      };
      if(android_id==null)android_id=Secure.getString(getContentResolver(), Secure.ANDROID_ID);
      if(wake_lock==null)
      {
         PowerManager power_manager=(PowerManager)getSystemService(POWER_SERVICE); if(power_manager!=null)
         {
            wake_lock=power_manager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, ""); // will crash if null is used
            if(wake_lock!=null)wake_lock.setReferenceCounted(false); // disable ref counting so one 'release' will be immediate
         }
      }
      if(wifi_lock==null)
      {
         WifiManager wifi_manager=(WifiManager)getSystemService(WIFI_SERVICE); if(wifi_manager!=null)
         {
            wifi_lock=wifi_manager.createWifiLock(WifiManager.WIFI_MODE_FULL_HIGH_PERF, ""); // will crash if null is used
            if(wifi_lock!=null)wifi_lock.setReferenceCounted(false); // disable ref counting so one 'release' will be immediate
         }
      }

      super.onCreate(savedInstanceState);

      startService(new Intent(this, DetectForceKill.class)); // start service that detects force kill

      initIAB();
      initFB (savedInstanceState);
      com.esenthel.Native.bannerSize(0, 0); // at this stage the banner is lost, but the banner size on the native size is still present, so reset it
      if(ad_request==null)setAdRequest(false);
      initChartboost();
      initNotification();

      /*log("android_id "+android_id);
      log("BOARD "+Build.BOARD);
      log("BOOTLOADER "+Build.BOOTLOADER);
      log("BRAND "+Build.BRAND);
      log("DEVICE "+Build.DEVICE);
      log("DISPLAY "+Build.DISPLAY);
      log("FINGERPRINT "+Build.FINGERPRINT);
      log("HARDWARE "+Build.HARDWARE);
      log("HOST "+Build.HOST);
      log("ID "+Build.ID);
      log("MANUFACTURER "+Build.MANUFACTURER);
      log("MODEL "+Build.MODEL);
      log("PRODUCT "+Build.PRODUCT);
      log("SERIAL "+Build.SERIAL);
      log("TAGS "+Build.TAGS);
      log("TIME "+Build.TIME);
      log("TYPE "+Build.TYPE);
      log("USER "+Build.USER);*/
   }
   @Override protected final void onActivityResult(int requestCode, int resultCode, Intent data)
   {
      super.onActivityResult(requestCode, resultCode, data);
      Session session=Session.getActiveSession(); if(  session!=null)  session.onActivityResult(this, requestCode, resultCode, data);
                                                  if(ui_helper!=null)ui_helper.onActivityResult(      requestCode, resultCode, data, fb_dialog_callback);
      switch(requestCode)
      {
         case REQUEST_CODE_IAB:
         {
            int    responseCode=-1;
            long   date=0;
            String purchaseData=null, sku=null, custom_data=null, token=null, signature=null;
            if(data!=null)
            {
               // data.getExtras()
               responseCode=getResponseCodeFromIntent(data);
               purchaseData=data.getStringExtra(RESPONSE_INAPP_PURCHASE_DATA);
               signature   =data.getStringExtra(RESPONSE_INAPP_SIGNATURE);
               if(purchaseData!=null)
               {
                  try
                  {
                     JSONObject o=new JSONObject(purchaseData);
                   //o.optString("orderId");
                   //o.optString("packageName");
                 sku=o.optString("productId");
                date=o.optLong("purchaseTime");
                   //o.optInt("purchaseState");
         custom_data=o.optString("developerPayload");
               token=o.optString("token", o.optString("purchaseToken"));
                  }catch(Exception exception){} // failed to parse data
               }
            }

            int result=billingToEE(responseCode);
            if( result==RES_OK     )result=((resultCode==Activity.RESULT_OK && sku!=null) ? (verifyPurchase(purchaseData, signature) ? RES_PURCHASED : RES_VERIFICATION_FAIL) : RES_UNKNOWN);
            if( result==RES_UNKNOWN)if(resultCode==Activity.RESULT_CANCELED)result=RES_SERVICE_CANCELED;
          //log("responseCode:"+responseCode+", resultCode:"+resultCode+", result:"+result);

            com.esenthel.Native.purchased(result, sku, custom_data, token, date);
         }break;
      }
   }
   @Override public final void onConfigurationChanged(Configuration newConfig)
   {
      super.onConfigurationChanged(newConfig);
      adPos(banner_x, banner_y); // reset banner position in new screen size/orientation
   }
   public static final void messageBox(String title, String text, boolean exit)
   {
      if(handler!=null)
      {
         Message message=handler.obtainMessage(); if(message!=null)
         {
            Bundle bundle=new Bundle(); if(bundle!=null)
            {
                       bundle.putString ("title", title);
                       bundle.putString ("text" , text );
               if(exit)bundle.putBoolean("exit" , exit );
               message.setData(bundle);
               handler.sendMessage(message);
            }
         }
      }
   }
   public static final void toast(final String text)
   {
      Handler handler=new Handler(Looper.getMainLooper());
      handler.post(new Runnable()
      {
         @Override public final void run()
         {
            Toast.makeText(context, text, Toast.LENGTH_LONG).show();
         }
      });
   }
   public static final void stayAwake(final int mode)
   {
      if(wake_lock!=null)
      {
         if(mode==AWAKE_SYSTEM)wake_lock.acquire();
         else                  wake_lock.release();
      }
      if(wifi_lock!=null)
      {
         if(mode==AWAKE_SYSTEM)wifi_lock.acquire();
         else                  wifi_lock.release();
      }
      if(activity!=null)activity.runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            Window window=activity.getWindow();
            if(window!=null)window.setFlags((mode==AWAKE_SCREEN) ? WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON : 0, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
         }
      });
   }
   public static final boolean openAppSettings()
   {
      Intent intent=new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
      intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
      intent.setData(Uri.fromParts("package", "EE_PACKAGE", null));
      boolean ok=true; try{context.startActivity(intent);}catch(Exception exception){ok=false;}
      return  ok;
   }
   public static final long driveSizeFree (String path) {File file=new File(path); if(file!=null && file.exists())return file.getUsableSpace(); return -1;}
   public static final long driveSizeTotal(String path) {File file=new File(path); if(file!=null && file.exists())return file.getTotalSpace (); return -1;}

   public static final String manufacturer() {return Build.MANUFACTURER;}
   public static final String model       () {return Build.MODEL;}
   public static final String serial      () {return Build.SERIAL;}
   public static final String androidID   () {return android_id;}

   /******************************************************************************/
   // KEYBOARD
   /******************************************************************************/
   @Override public final boolean dispatchKeyEvent(KeyEvent event)
   {
      if(event!=null)if(event.getAction()!=KeyEvent.ACTION_UP)
      {
         int key_code=event.getKeyCode    (); // use 'getKeyCode' instead of 'getScanCode' to match 'AKeyEvent_getKeyCode' on native C++ side
         int unicode =event.getUnicodeChar(); if(unicode!=0)com.esenthel.Native.key(unicode, key_code);else
         {
            String s=event.getCharacters(); if(s!=null)for(int i=0; i<s.length(); i++)com.esenthel.Native.key(s.codePointAt(i), key_code);
         }
      }
      return super.dispatchKeyEvent(event);
   }
   public final void editTextSetDo(String text, int start, int end, boolean password)
   {
      if(edit_text!=null)
      {
         edit_text.removeTextChangedListener(text_watcher);
         edit_text.setText(text);
         edit_text.setSelection(start, end);
         edit_text.setInputType(password ? InputType.TYPE_CLASS_TEXT|InputType.TYPE_TEXT_FLAG_MULTI_LINE|InputType.TYPE_TEXT_VARIATION_PASSWORD : InputType.TYPE_CLASS_TEXT|InputType.TYPE_TEXT_FLAG_MULTI_LINE); // TYPE_TEXT_FLAG_MULTI_LINE prevents from Enter making the keyboard disappear (on Google and Samsung keyboards)
         edit_text.addTextChangedListener(text_watcher);
      }
   }
   public final void editTextSet(final String text, final int start, final int end, final boolean password)
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            editTextSetDo(text, start, end, password);
         }
      });
   }
   public final void editText(final String text, final int start, final int end, final boolean password)
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            if(edit_text==null)
            {
               if(text_watcher==null)text_watcher=new TextWatcher()
               {
                  @Override public final void afterTextChanged(Editable s)
                  {
                     if(s!=null)
                     {
                        int start=0, end=0;
                        if(edit_text!=null)
                        {
                           start=edit_text.getSelectionStart();
                           end  =edit_text.getSelectionEnd  ();
                        }
                        com.esenthel.Native.text(s.toString(), start, end);
                     }
                  }
                  @Override public final void beforeTextChanged(CharSequence s, int start, int count , int after) {}
                  @Override public final void     onTextChanged(CharSequence s, int start, int before, int count) {}
               };

               FrameLayout.LayoutParams layout=new FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT);
               edit_text=new EditText(activity);
               edit_text.setLayoutParams(layout);
               edit_text.setImeOptions(edit_text.getImeOptions()|EditorInfo.IME_FLAG_NO_FULLSCREEN);
               activity.addContentView(edit_text, layout);
            }
            editTextSetDo(text, start, end, password);
            edit_text.setVisibility(View.VISIBLE);
            edit_text.bringToFront();
            edit_text.requestFocus();

            InputMethodManager imm=(InputMethodManager)activity.getSystemService(Context.INPUT_METHOD_SERVICE);
            imm.showSoftInput(edit_text, 0); // use 0 instead of 'SHOW_FORCED' because that one will keep keyboard visible even when app is hidden using the Home button
         }
      });
   }
   public final void editTextHide()
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            Window window=activity.getWindow(); if(window!=null)
            {
               View view=window.getDecorView(); if(view!=null)
               {
                  InputMethodManager imm=(InputMethodManager)activity.getSystemService(Activity.INPUT_METHOD_SERVICE);
                  imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
               }
            }
            if(edit_text!=null)edit_text.setVisibility(View.GONE);
         }
      });
   }
   /******************************************************************************/
   // MICROPHONE
   /******************************************************************************/
   public static class EsenthelAudioRecord extends AudioRecord
   {
      public Thread thread;

      EsenthelAudioRecord(int frequency, int channelConfig, int audioFormat, int minBufferSize)
      {
         super(MediaRecorder.AudioSource.MIC, frequency, channelConfig, audioFormat, minBufferSize);
      }
   }
   public static final boolean hasAudioRecord()
   {
      return context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_MICROPHONE);
   }
   public static final void delAudioRecord(EsenthelAudioRecord audioRecord)
   {
      audioRecord.stop(); // first stop recording
      try{audioRecord.thread.join();}catch(Exception exception){} // now wait for thread to finish
      audioRecord.thread=null;
      audioRecord.release();
   }
   public static final EsenthelAudioRecord newAudioRecord(final long sound_record, int bits, int channels, int frequency)
   {
      if(channels>=1 && channels<=2 && (bits==8 || bits==16))
      {
         int channelConfig  =((channels==1) ? AudioFormat.CHANNEL_IN_MONO   : AudioFormat.CHANNEL_IN_STEREO ),
             audioFormat    =((bits    ==8) ? AudioFormat.ENCODING_PCM_8BIT : AudioFormat.ENCODING_PCM_16BIT),
             min_buffer_size=AudioRecord.getMinBufferSize(frequency, channelConfig, audioFormat); // for 44100 hz, 1 channel, 16 bits, Galaxy Note 4 returns 3840
         if( min_buffer_size>0)
         {
            try
            {
               final int poll_size=min_buffer_size; // this will affect latency, so don't increase this
               final EsenthelAudioRecord audioRecord=new EsenthelAudioRecord(frequency, channelConfig, audioFormat, min_buffer_size*2); // mul by 2 because "this size doesn't guarantee a smooth recording under load, and higher values should be chosen according to the expected frequency at which the AudioRecord instance will be polled for new data" - http://developer.android.com/reference/android/media/AudioRecord.html#getMinBufferSize(int, int, int)
               if(audioRecord.getState()==AudioRecord.STATE_INITIALIZED)
               {
                  audioRecord.startRecording(); // start recording before starting the thread, because inside it we're checking for RECORDSTATE_RECORDING
                  audioRecord.thread=new Thread()
                  {
                     @Override public final void run()
                     {
                        ByteBuffer buf=ByteBuffer.allocateDirect(poll_size);
                        if(buf!=null)for(; audioRecord.getRecordingState()==AudioRecord.RECORDSTATE_RECORDING; )
                        {
                           int read=audioRecord.read(buf, poll_size); // this is a blocking call, it will wait until data is available
                           if( read>0)com.esenthel.Native.audioRecord(sound_record, buf, read);else
                           if( read<0)break; // error
                        }
                     }
                  };
                  audioRecord.thread.setPriority(Thread.MAX_PRIORITY);
                  audioRecord.thread.start();
                  return audioRecord;
               }
            }catch(Exception exception){}
         }
      }
      return null;
   }
   /******************************************************************************/
   // ADMOB
   /******************************************************************************/
   public static final void setAdRequest(boolean test_mode)
   {
      AdRequest.Builder builder=new AdRequest.Builder().addTestDevice(AdRequest.DEVICE_ID_EMULATOR);
      if(test_mode)
      {
         String android_id_md5;
         try
         {
            MessageDigest md5=MessageDigest.getInstance("MD5");
            md5.update(android_id.getBytes(), 0, android_id.length());
            android_id_md5=String.format("%032X", new Object[]{new BigInteger(1, md5.digest())});
         }catch(Exception exception){android_id_md5=android_id.substring(0, 32);}
         builder.addTestDevice(android_id_md5);
      }
      ad_request=builder.build();
   }
   public final void bannerHide()
   {
      if(popup_window!=null)
      {
         popup_window.dismiss();
         popup_window=null;
      }
      if(popup_window_layout!=null)
      {
         ViewGroup parent=(ViewGroup)popup_window_layout.getParent(); if(parent!=null)parent.removeView(popup_window_layout);
         popup_window_layout.removeAllViews();
         popup_window_layout=null;
      }
      if(ad_view_layout!=null)
      {
         ViewGroup parent=(ViewGroup)ad_view_layout.getParent(); if(parent!=null)parent.removeView(ad_view_layout);
         ad_view_layout.removeAllViews();
         ad_view_layout=null;
      }
   }
   public final void adViewDel()
   {
      if(ad_view!=null)
      {
         ad_view.destroy(); ad_view=null; ad_view_loaded=false;
      }
   }
   public final void adDel(final boolean banner)
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            if(banner)
            {
               banner_id=null;
               bannerHide();
               adViewDel();
            }else
            {
               inters_id=null;
               interstitial=null;
            }
         }
      });
   }
   public final void adCreate(final String unit_id, final int type)
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            if(ad_request!=null)
            {
               AdSize ad_size=null;
               switch(type)
               {
                  case AD_BANNER          : ad_size=AdSize.BANNER          ; break;
                  case AD_MEDIUM_RECTANGLE: ad_size=AdSize.MEDIUM_RECTANGLE; break;
                  case AD_FULL_BANNER     : ad_size=AdSize.FULL_BANNER     ; break;
                  case AD_LEADERBOARD     : ad_size=AdSize.LEADERBOARD     ; break;
                  case AD_SMART_BANNER    : ad_size=AdSize.SMART_BANNER    ; break;
                  case AD_INTERSTITIAL    :
                  {
                     inters_id=unit_id;
                     interstitial=new InterstitialAd(activity);
                     interstitial.setAdUnitId(unit_id);
                     interstitial.setAdListener(new AdListener()
                     {
                        @Override public final void onAdLoaded         (             ) {com.esenthel.Native.adState(false, AD_DONE   ); if(inters_show){inters_show=false; adVisible(false, true);}}
                        @Override public final void onAdFailedToLoad   (int errorCode) {com.esenthel.Native.adState(false, AD_ERROR  );}
                        @Override public final void onAdOpened         (             ) {}
                        @Override public final void onAdClosed         (             ) {com.esenthel.Native.adState(false, AD_LOADING); adCreate(unit_id, type);} // when ad is closed, it cannot be displayed again, so reload again
                        @Override public final void onAdLeftApplication(             ) {}
                     });
                     interstitial.loadAd(ad_request);
                  }break;
               }
               if(ad_size!=null)
               {
                  banner_id  =unit_id;
                  banner_type=type;
                  bannerHide();
                  adViewDel();
                  ad_view=new AdView(activity);
                  ad_view.setAdSize(ad_size);
                  ad_view.setAdUnitId(unit_id);
                  ad_view.setAdListener(new AdListener()
                  {
                     @Override public final void onAdFailedToLoad   (int errorCode) {com.esenthel.Native.adState(true, AD_ERROR);}
                     @Override public final void onAdOpened         (             ) {}
                     @Override public final void onAdClosed         (             ) {}
                     @Override public final void onAdLeftApplication(             ) {}
                     @Override public final void onAdLoaded         (             )
                     {
                        if(ad_view!=null)
                        {
                           AdSize ad_size=ad_view.getAdSize();
                           if(ad_size!=null)com.esenthel.Native.bannerSize(ad_size.getWidthInPixels (activity),
                                                                           ad_size.getHeightInPixels(activity));
                        }
                        com.esenthel.Native.adState(true, AD_DONE);
                        ad_view_loaded=true; if(banner_visible)adVisible(true, true);
                     }
                  });
                  ad_view.loadAd(ad_request);
               }
            }
         }
      });
   }
   public final float dipToPx(float f) {return f*getResources().getDisplayMetrics().density;}
   public final float pxToDip(float f) {return f/getResources().getDisplayMetrics().density;}
   public final int screenW() {return getWindowManager().getDefaultDisplay().getWidth ();}
   public final int screenH() {return getWindowManager().getDefaultDisplay().getHeight();}
   public final int popupWindowW() {return (popup_window!=null) ? (int)dipToPx(popup_window.getWidth ()) : 0;}
   public final int popupWindowH() {return (popup_window!=null) ? (int)dipToPx(popup_window.getHeight()) : 0;}
   public final int bannerX() {int w=popupWindowW(); return (banner_x<0) ? 0 : (banner_x>0) ? screenW()-w : (screenW()-w)/2;}
   public final int bannerY() {int h=popupWindowH(); return (banner_y>0) ? 0 : (banner_y<0) ? screenH()-h : (screenH()-h)/2;}
   public final void adVisible(final boolean banner, final boolean visible)
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            if(banner) // operate on banner
            {
               banner_visible=visible;
               if(visible)
               {
                  if(ad_view!=null && ad_view_loaded)
                  {
                     AdSize ad_size=ad_view.getAdSize();
                     if(ad_size!=null && popup_window==null)
                     {
                        MarginLayoutParams params=new MarginLayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
                        params.setMargins(0, 0, 0, 0);
                        popup_window=new PopupWindow(activity);
                        popup_window.setWindowLayoutMode(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
                             ad_view_layout=new LinearLayout(activity);
                        popup_window_layout=new LinearLayout(activity);
                        ad_view_layout.setPadding(-11, -11, -11, -11); // these values are required for ads to display on high resolution screens like 2560x1440 (Galaxy Note 4), this was also tested on 1280x720 (Galaxy Note 2), if values are bigger then the ad will not display complaining it has not enough space
                          popup_window.setContentView(     ad_view_layout);
                              activity.setContentView(popup_window_layout, params);
                        ad_view_layout.addView  (ad_view, params);
                          popup_window.setWidth (ad_size.getWidth ());
                          popup_window.setHeight(ad_size.getHeight());
                          popup_window.showAtLocation(popup_window_layout, Gravity.LEFT|Gravity.TOP, bannerX(), bannerY());
                          popup_window.update();
                     }
                  }
               }else
               {
                  bannerHide();
               }
            }else // operate on interstitial
            {
               if(visible)
               {
                  if(interstitial!=null && interstitial.isLoaded())interstitial.show();
                  else                                             inters_show=true; // show next time
               }else
               {
                  inters_show=false; // disable showing next time
                  // there's no method to hide
               }
            }
         }
      });
   }
   public final void adPos(int x, int y)
   {
      banner_x=x;
      banner_y=y;
      if(popup_window!=null)runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            if(popup_window!=null)popup_window.update(bannerX(), bannerY(), -1, -1);
         }
      });
   }

   /******************************************************************************/
   // IAB - In App Billing
   /******************************************************************************/
   final void initIAB()
   {
      if(iab_service_conn==null)
      {
         iab_service_conn=new ServiceConnection()
         {
            @Override public final void onServiceDisconnected(ComponentName name) {iab_service=null;}

            @Override public final void onServiceConnected(ComponentName name, IBinder binder)
            {
               iab_service=IInAppBillingService.Stub.asInterface(binder);
               boolean inapp_supported=false, subs_supported=false;
               try
               {
                  inapp_supported=(iab_service.isBillingSupported(3, "EE_PACKAGE", ITEM_TYPE_INAPP)==BILLING_RESPONSE_RESULT_OK);
                   subs_supported=(iab_service.isBillingSupported(3, "EE_PACKAGE", ITEM_TYPE_SUBS )==BILLING_RESPONSE_RESULT_OK);
               }catch(Exception exception){}
               com.esenthel.Native.connected(inapp_supported, subs_supported);
            }
         };

         Intent serviceIntent=new Intent("com.android.vending.billing.InAppBillingService.BIND");
         serviceIntent.setPackage("com.android.vending");
         List<ResolveInfo> services=getPackageManager().queryIntentServices(serviceIntent, 0);
         if(services!=null && !services.isEmpty())bindService(serviceIntent, iab_service_conn, Context.BIND_AUTO_CREATE);
      }
   }
   final void shutIAB()
   {
      if(iab_service_conn!=null){unbindService(iab_service_conn); iab_service_conn=null;}
   }
   static final int billingToEE(int result)
   {
      switch(result)
      {
         case BILLING_RESPONSE_RESULT_OK                 : return RES_OK;
         case BILLING_RESPONSE_RESULT_USER_CANCELED      : return RES_USER_CANCELED;
         case BILLING_RESPONSE_RESULT_BILLING_UNAVAILABLE: return RES_SERVICE_UNAVAILABLE;
         case BILLING_RESPONSE_RESULT_ITEM_UNAVAILABLE   : return RES_ITEM_UNAVAILABLE;
         case BILLING_RESPONSE_RESULT_ITEM_ALREADY_OWNED : return RES_ALREADY_OWNED;
         case BILLING_RESPONSE_RESULT_ITEM_NOT_OWNED     : return RES_NOT_OWNED;
         default                                         : return RES_UNKNOWN;
      }
   }
   static final int getResponseCodeFromObject(Object o)
   {
      if(o==null)return BILLING_RESPONSE_RESULT_OK;
      if(o instanceof Integer)return      ((Integer)o). intValue();
      if(o instanceof Long   )return (int)((Long   )o).longValue();
      return BILLING_RESPONSE_RESULT_ERROR; // Unexpected type for bundle response code
   }
   static final int getResponseCodeFromBundle(Bundle b) {return getResponseCodeFromObject(b.            get(RESPONSE_CODE));}
   static final int getResponseCodeFromIntent(Intent i) {return getResponseCodeFromObject(i.getExtras().get(RESPONSE_CODE));}

   static final PublicKey generatePublicKey(String encodedPublicKey)
   {
      if(Is(encodedPublicKey))
      {
         try
         {
            byte[] decodedKey=com.esenthel.Base64.decode(encodedPublicKey); if(decodedKey==null)return null;
            KeyFactory keyFactory=KeyFactory.getInstance(KEY_FACTORY_ALGORITHM);
            return keyFactory.generatePublic(new X509EncodedKeySpec(decodedKey));
         }catch(Exception exception){}
      }
      return null;
   }
   static final boolean verifyPurchase(String data, String signature)
   {
      if(license_key==null)
      {
         if(!Is("EE_LICENSE_KEY"))return true; // no license key specified, so always succeed
         license_key=generatePublicKey("EE_LICENSE_KEY");
         if(license_key==null)return false; // failed to generate license key
      }
      if(!Is(data) || !Is(signature))return false;
      try
      {
         Signature sig=Signature.getInstance(SIGNATURE_ALGORITHM);
         sig.initVerify(license_key);
         sig.update(data.getBytes());
         byte[] decodedKey=com.esenthel.Base64.decode(signature); if(decodedKey==null)return false;
         if(!sig.verify(decodedKey))return false;
         return true;
      }catch(Exception exception){}
      return false;
   }

   public static final boolean getItemDetails(boolean regular_items, boolean subs, int item_ids_elms, long item_ids, long user)
   {
      if(iab_service==null)return false;

      ArrayList<String> skuList=new ArrayList<String>();
      for(int i=0; i<item_ids_elms; i++)skuList.add(com.esenthel.Native.getStr(item_ids, i));
      Bundle querySkus=new Bundle(); querySkus.putStringArrayList(GET_SKU_DETAILS_ITEM_LIST, skuList);

      for(int i=0; i<2; i++)
      {
         boolean sub=(i!=0);
         if(sub ? subs : regular_items)
         {
            String itemType=(sub ? ITEM_TYPE_SUBS : ITEM_TYPE_INAPP);
            try
            {
               Bundle skuDetails=iab_service.getSkuDetails(3, "EE_PACKAGE", itemType, querySkus);
               if(skuDetails.containsKey(RESPONSE_GET_SKU_DETAILS_LIST))
               {
                  ArrayList<String> responseList=skuDetails.getStringArrayList(RESPONSE_GET_SKU_DETAILS_LIST);
                  for(String json:responseList)
                  {
                     String sku, name, desc, price;
                     try
                     {
                        JSONObject o=new JSONObject(json);
                        sku  =o.optString("productId");
                            //o.optString("type");
                        price=o.optString("price");
                        name =o.optString("title");
                        desc =o.optString("description");
                     }catch(Exception exception){continue;} // failed to parse data, proceed to next one
                     if(sku!=null)com.esenthel.Native.listItem(user, sku, name, desc, price, sub);
                  }
               }
            }catch(Exception exception){}
         }
      }
      return true;
   }

   public static final boolean getPurchases(boolean sub, long user)
   {
      if(iab_service==null)return false;
      String itemType=(sub ? ITEM_TYPE_SUBS : ITEM_TYPE_INAPP), continueToken=null;
      do
      {
         try
         {
            Bundle ownedItems=iab_service.getPurchases(3, "EE_PACKAGE", itemType, continueToken);
            int response =getResponseCodeFromBundle(ownedItems);
            if( response!=BILLING_RESPONSE_RESULT_OK)return false;

            if(!ownedItems.containsKey(RESPONSE_INAPP_ITEM_LIST         )
            || !ownedItems.containsKey(RESPONSE_INAPP_PURCHASE_DATA_LIST)
            || !ownedItems.containsKey(RESPONSE_INAPP_SIGNATURE_LIST    ))return false; // Bundle doesn't contain required fields

            ArrayList<String> ownedSkus       =ownedItems.getStringArrayList(RESPONSE_INAPP_ITEM_LIST);
            ArrayList<String> purchaseDataList=ownedItems.getStringArrayList(RESPONSE_INAPP_PURCHASE_DATA_LIST);
            ArrayList<String> signatureList   =ownedItems.getStringArrayList(RESPONSE_INAPP_SIGNATURE_LIST);

            for(int i=0; i<purchaseDataList.size(); i++)
            {
               String purchaseData=purchaseDataList.get(i),
                      signature   =signatureList   .get(i),
                      sku         =ownedSkus       .get(i),
                      custom_data=null, token=null;
               long   date        =0;
               if(purchaseData!=null)
               {
                  try
                  {
                     JSONObject o=new JSONObject(purchaseData);
                   //o.optString("orderId");
                   //o.optString("packageName");
               //sku=o.optString("productId");
                date=o.optLong("purchaseTime");
                   //o.optInt("purchaseState");
         custom_data=o.optString("developerPayload");
               token=o.optString("token", o.optString("purchaseToken"));
                  }catch(Exception exception){} // failed to parse data
               }
               if(verifyPurchase(purchaseData, signature))com.esenthel.Native.listPurchase(user, sku, custom_data, token, date);
            }
            continueToken=ownedItems.getString(INAPP_CONTINUATION_TOKEN);
         }catch(Exception exception){return false;}
      }while(Is(continueToken));

      return true;
   }

   public static final int buy(String id, String data, boolean subscription)
   {
      if(iab_service==null || activity==null)return RES_SERVICE_UNAVAILABLE;
      int result;
      try
      {
         Bundle buyIntentBundle=iab_service.getBuyIntent(3, "EE_PACKAGE", id, subscription ? ITEM_TYPE_SUBS : ITEM_TYPE_INAPP, data);
         result=billingToEE(getResponseCodeFromBundle(buyIntentBundle));
         if(result==RES_OK)
         {
            result=RES_WAITING; // we're not done yet
            PendingIntent pendingIntent=buyIntentBundle.getParcelable(RESPONSE_BUY_INTENT);
            activity.startIntentSenderForResult(pendingIntent.getIntentSender(), REQUEST_CODE_IAB, new Intent(), Integer.valueOf(0), Integer.valueOf(0), Integer.valueOf(0));
         }
      }catch(Exception exception){result=RES_UNKNOWN;}
      return result;
   }

   public static final int consume(String token)
   {
      if(iab_service==null)return RES_SERVICE_UNAVAILABLE;
      int result;
      try
      {
         result=billingToEE(iab_service.consumePurchase(3, "EE_PACKAGE", token));
         if(result==RES_OK)result=RES_CONSUMED;
      }catch(Exception exception){result=RES_UNKNOWN;}
      return result;
   }

   /******************************************************************************/
   // FB - Facebook
   /******************************************************************************/
  volatile static Bundle                 facebook_post;
   private static boolean                facebook_get_name=false, facebook_get_friends=false, facebook_want_post=false;
   private        UiLifecycleHelper      ui_helper;
   private final  Session.StatusCallback ui_helper_callback=new Session.StatusCallback()
   {
      @Override public final void call(Session session, SessionState state, Exception exception) {}
   };
   private final FacebookDialog.Callback fb_dialog_callback=new FacebookDialog.Callback()
   {
      @Override public final void onError   (FacebookDialog.PendingCall pendingCall, Exception error, Bundle data) {com.esenthel.Native.facebookPost(POST_ERROR);}
      @Override public final void onComplete(FacebookDialog.PendingCall pendingCall,                  Bundle data)
      {
         int result=POST_ERROR;
         if(data!=null)
         {
            String gesture=FacebookDialog.getNativeDialogCompletionGesture(data);
            if(gesture!=null)
            {
               if(gesture.equals("post"  ))result=POST_SUCCESS;else
               if(gesture.equals("cancel"))result=POST_CANCEL ;
            }else result=POST_NOT_LOGGED_IN;
         }
         com.esenthel.Native.facebookPost(result);
      }
   };
   private final Session.StatusCallback fb_status_callback=new Session.StatusCallback()
   {
      @Override public final void call(Session session, SessionState state, Exception exception)
      {
         if(facebookLoggedIn())
         {
            if(facebook_get_name   ){facebook_get_name   =false;                       facebookGetMeDo     ();}
            if(facebook_get_friends){facebook_get_friends=false;                       facebookGetFriendsDo();}
            if(facebook_want_post  ){facebook_want_post  =false; if(!facebookCanPost())facebookWantPost    ();}
                                                                                       facebookPostDo      ();
         }
      }
   };

   final void initFB(Bundle savedInstanceState)
   {
      com.facebook.Settings.addLoggingBehavior(LoggingBehavior.INCLUDE_ACCESS_TOKENS);
      Session session=Session.getActiveSession();
      if(session==null)
      {
         if(savedInstanceState!=null)session=Session.restoreSession(this, null, fb_status_callback, savedInstanceState);
         if(session==null)session=new Session(this);
         Session.setActiveSession(session);
         if(session.getState().equals(SessionState.CREATED_TOKEN_LOADED))session.openForRead(new Session.OpenRequest(this).setPermissions(Arrays.asList("public_profile", "email", "user_friends")).setCallback(fb_status_callback));
      }
      ui_helper=new UiLifecycleHelper(this, ui_helper_callback);
      ui_helper.onCreate(savedInstanceState);
   }
   public static final boolean facebookLoggedIn()
   {
      Session session=Session.getActiveSession(); return session!=null && session.isOpened();
   }
   public static final boolean facebookCanPost()
   {
      Session session=Session.getActiveSession(); return session!=null && session.getPermissions().contains("publish_actions");
   }
   public static final void facebookWantPost()
   {
      Session session=Session.getActiveSession(); if(session!=null)session.requestNewPublishPermissions(new Session.NewPermissionsRequest(activity, "publish_actions"));
   }
   public static final void facebookLogOut()
   {
      Session session=Session.getActiveSession(); if(session!=null && !session.isClosed())session.closeAndClearTokenInformation();
   }
   public final void facebookLogIn()
   {
      Session session=Session.getActiveSession();
      if(session==null || session.isClosed())
      {
         session=new Session(this);
         Session.setActiveSession(session);
      }
      if(!session.isOpened())session.openForRead(new Session.OpenRequest(this).setPermissions(Arrays.asList("public_profile", "email", "user_friends")).setCallback(fb_status_callback));
   }
   public static final void facebookGetMeDo()
   {
      if(facebookLoggedIn())Request.newMeRequest(Session.getActiveSession(), new Request.GraphUserCallback()
      {
         @Override public final void onCompleted(GraphUser user, Response response)
         {
            if(user!=null)com.esenthel.Native.facebookMe(user.getId(), user.getName(), (String)user.getProperty("email"));
         }
      }).executeAsync();
   }
   public static final void facebookGetFriendsDo()
   {
      if(facebookLoggedIn())
      {
         Request request=Request.newMyFriendsRequest(Session.getActiveSession(), new GraphUserListCallback()
         {
            @Override public final void onCompleted(List<GraphUser> users, Response response)
            {
               if(users!=null)
               {
                  ArrayList<String>   ids=new ArrayList<String>();
                  ArrayList<String> names=new ArrayList<String>();
                  for(int i=0; i<users.size(); i++)
                  {
                     GraphUser user=users.get(i);
                       ids.add(user.getId  ());
                     names.add(user.getName());
                  }
                  com.esenthel.Native.facebookFriends(ids, names);
               }
            }
         });
         Bundle params=new Bundle(); params.putString("fields", "id, name");
         request.setParameters(params);
         request.executeAsync();
      }
   }
   public static final void facebookPostDo() // this does not require POST permission as it uses built-in Facebook app or WebView which handle this permission
   {
      Bundle post=facebook_post; facebook_post=null; if(post!=null && facebookLoggedIn())
      {
         WebDialog dialog=(new WebDialog.FeedDialogBuilder(activity, Session.getActiveSession(), post))
            .setOnCompleteListener(new OnCompleteListener()
            {
               @Override public final void onComplete(Bundle values, com.facebook.FacebookException error)
               {
                  int result=POST_CANCEL;
                  if(error!=null)
                  {
                     if(!(error instanceof com.facebook.FacebookOperationCanceledException))result=POST_ERROR;
                  }else
                     if(values!=null && values.getString("post_id")!=null)result=POST_SUCCESS;
                  com.esenthel.Native.facebookPost(result);
               }
            }).build();
         dialog.show();
      }
   }
   public final void facebookGetMe     () {if(facebookLoggedIn())runOnUiThread(new Runnable(){@Override public final void run(){facebookGetMeDo     ();}});else{facebook_get_name   =true; facebookLogIn();}}
   public final void facebookGetFriends() {if(facebookLoggedIn())runOnUiThread(new Runnable(){@Override public final void run(){facebookGetFriendsDo();}});else{facebook_get_friends=true; facebookLogIn();}}

   public final void facebookPost(String message, String url, String image_url, String title, String desc, String caption)
   {
      if(ui_helper!=null && FacebookDialog.canPresentShareDialog(this, FacebookDialog.ShareDialogFeature.SHARE_DIALOG)) // this does not require being logged in
      {
         FacebookDialog shareDialog=new FacebookDialog.ShareDialogBuilder(this)
            .setName       (title)
            .setLink       (url)
            .setPicture    (image_url)
            .setDescription(desc)
            .setCaption    (caption)
            .build();
         ui_helper.trackPendingDialogCall(shareDialog.present());
      }else
      {
         Bundle post=new Bundle();
         post.putString("message"    , message);
         post.putString("link"       , url);
         post.putString("picture"    , image_url);
         post.putString("name"       , title);
         post.putString("description", desc);
         post.putString("caption"    , caption);
         facebook_post=post;
         if(facebookLoggedIn())runOnUiThread(new Runnable(){@Override public final void run(){facebookPostDo();}});else facebookLogIn();
      }
   }

   /******************************************************************************/
   // CHARTBOOST
   /******************************************************************************/
   public static final boolean chartboostVisible() {return Chartboost.isAnyViewVisible();}

   public static final void    chartboostInterstitialLoad     () {       Chartboost.cacheInterstitial(CBLocation.LOCATION_DEFAULT);}
   public static final void    chartboostInterstitialShow     () {       Chartboost. showInterstitial(CBLocation.LOCATION_DEFAULT);}
   public static final boolean chartboostInterstitialAvailable() {return Chartboost.  hasInterstitial(CBLocation.LOCATION_DEFAULT);}

   public static final void    chartboostRewardedVideoLoad     () {       Chartboost.cacheRewardedVideo(CBLocation.LOCATION_DEFAULT);}
   public static final void    chartboostRewardedVideoShow     () {       Chartboost. showRewardedVideo(CBLocation.LOCATION_DEFAULT);}
   public static final boolean chartboostRewardedVideoAvailable() {return Chartboost.  hasRewardedVideo(CBLocation.LOCATION_DEFAULT);}

   final void initChartboost()
   {
      if(EE_INIT_CHARTBOOST)
      {
         Chartboost.startWithAppId(this, "EE_CHARTBOOST_APP_ID", "EE_CHARTBOOST_APP_SIGNATURE");
         Chartboost.setDelegate(new ChartboostDelegate() { // it must be called between 'Chartboost.startWithAppId' and 'Chartboost.onCreate'
		    //@Override public final boolean shouldRequestInterstitial(String location) {return true;}
		    //@Override public final boolean shouldDisplayInterstitial(String location) {return true;}
		      @Override public final void didCacheInterstitial(String location) {com.esenthel.Native.chartboost(INTERSTITIAL_LOADED);}
		      @Override public final void didFailToLoadInterstitial(String location, CBImpressionError error) {com.esenthel.Native.chartboost(INTERSTITIAL_LOAD_FAIL);}
		      @Override public final void didDisplayInterstitial(String location) {com.esenthel.Native.chartboost(INTERSTITIAL_DISPLAYED);}
		      @Override public final void didDismissInterstitial(String location) {com.esenthel.Native.chartboost(INTERSTITIAL_CLOSED);}
		    //@Override public final void didCloseInterstitial(String location) {com.esenthel.Native.chartboost(INTERSTITIAL_CLOSED);} this doesn't get called when interstitial is closed through clicking
		      @Override public final void didClickInterstitial(String location) {com.esenthel.Native.chartboost(INTERSTITIAL_CLICKED);}

		    //@Override public final boolean shouldDisplayRewardedVideo(String location) {return true;}
		      @Override public final void didCacheRewardedVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_LOADED);}
		      @Override public final void didFailToLoadRewardedVideo(String location, CBImpressionError error) {com.esenthel.Native.chartboost(REWARDED_VIDEO_LOAD_FAIL);}
		      @Override public final void didDisplayRewardedVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_DISPLAYED);}
		    //@Override public final void willDisplayVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_DISPLAYED);} this gets called everytime 'didDisplayRewardedVideo' gets called too, no need for 2 notifications
		      @Override public final void didDismissRewardedVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_CLOSED);}
		    //@Override public final void didCloseRewardedVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_CLOSED);} this gets called everytime 'didDismissRewardedVideo' gets called too, no need for 2 notifications
		      @Override public final void didClickRewardedVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_CLICKED);}
		      @Override public final void didCompleteRewardedVideo(String location, int reward) {com.esenthel.Native.chartboost(REWARDED_VIDEO_COMPLETED);}
         });
         Chartboost.setImpressionsUseActivities(true); // this is required due to OpenGL usage, it must be called between 'Chartboost.startWithAppId' and 'Chartboost.onCreate'
         Chartboost.onCreate(this);
      }
   }

   /******************************************************************************/
   // NOTIFICATIONS
   /******************************************************************************/
   public static class NotificationDismissed extends BroadcastReceiver
   {
      @Override public final void onReceive(Context context, Intent intent) {checkNotification(intent, false);}
   }
   public static final String            NOTIFICATION_CHANNEL_ID="ID";
   public static final String            NOTIFICATION_ID        ="EsenthelNotificationID";
   public static final int    BACKGROUND_NOTIFICATION_ID        =-1; // must not be 0 because 'startForeground' does not accept 0, use -1 because 0..+ are used by custom Notifications !! if this is changed then adjust 'Application.backgroundText' !!
   public static final void initNotification()
   {
      if(Build.VERSION.SDK_INT>=26)
      {
       /*Use this code and below 'getNotification'
         NotificationManager manager=(NotificationManager)activity.getSystemService(Context.NOTIFICATION_SERVICE);
         if(manager!=null)
         {
            android.app.NotificationChannel channel=new android.app.NotificationChannel(NOTIFICATION_CHANNEL_ID, "Name", NotificationManager.IMPORTANCE_DEFAULT);
          //channel.setDescription("Description");
            manager.createNotificationChannel(channel);
         }*/
      }
      if(Build.VERSION.SDK_INT>=23)
      {
         //NotificationManager manager=(NotificationManager)activity.getSystemService(Context.NOTIFICATION_SERVICE);
         //android.service.notification.StatusBarNotification[] notifications=manager.getActiveNotifications();
      }
   }
   public static final void checkNotification(Intent intent, boolean selected)
   {
      if(intent!=null)
      {
         int notification_id=intent.getIntExtra(NOTIFICATION_ID, -1); // -1=default value
         if( notification_id>=0)com.esenthel.Native.notification(notification_id, selected);
      }   
   }
   public static final Notification getNotification(int id, String title, String text, boolean dismissable)
   {
      Notification.Builder builder;
    //if(Build.VERSION.SDK_INT>=26)builder=new Notification.Builder(context, NOTIFICATION_CHANNEL_ID);else
                                   builder=new Notification.Builder(context);
      if(builder!=null)
      {
         Intent intent=new Intent(context, EsenthelActivity.class);
         if(id>=0)intent.putExtra   (NOTIFICATION_ID, id); // store NOTIFICATION_ID only for regular notifications
                  intent.addCategory(Intent.CATEGORY_LAUNCHER);
                  intent.setAction  (Intent.ACTION_MAIN);
         if(dismissable && id>=0)
         {
            Intent intent_dismiss=new Intent(context, NotificationDismissed.class);
            intent_dismiss.putExtra(NOTIFICATION_ID, id);
            PendingIntent pending_intent_dismiss=PendingIntent.getBroadcast(context, id*2+1, intent_dismiss, PendingIntent.FLAG_UPDATE_CURRENT); // use id*2+1 to make unique
            builder.setDeleteIntent(pending_intent_dismiss);
         }
         PendingIntent pending_intent=PendingIntent.getActivity(context, id*2, intent, PendingIntent.FLAG_UPDATE_CURRENT); // use id*2 to make unique
         return builder.setContentTitle(title).setContentText(text).setSmallIcon(R.drawable.notification).setLargeIcon(BitmapFactory.decodeResource(context.getResources(), R.drawable.icon)).setOngoing(!dismissable).setContentIntent(pending_intent).build();
      }
      return null;
   }
   public static final void setNotification(int id, String title, String text, boolean dismissable)
   {
      if(id==BACKGROUND_NOTIFICATION_ID && background_intent==null)return; // don't adjust background notification if we're not actually in background (can happen if we call this in InitPre, when application hasn't finished initializing but it's also not in the background)
      NotificationManager manager=(NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE); if(manager!=null)
      {
         Notification notification=getNotification(id, title, text, dismissable);
         if(notification!=null)manager.notify(id, notification);
      }
   }
   public static final void removeNotification(int id)
   {
      NotificationManager manager=(NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE); if(manager!=null)manager.cancel(id);
   }
   public static final void removeNotifications()
   {
      NotificationManager manager=(NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE); if(manager!=null)manager.cancelAll();
   }

   /******************************************************************************/
   // BACKGROUND SERVICE
   /******************************************************************************/
   public static class BackgroundService extends Service // !! if changing name of this class then update AndroidManifest.xml !!
   {
      @Override public final IBinder onBind        (Intent intent) {return null;}
    //@Override public final void    onCreate      (             ) {super.onCreate(); background_service=this;}
    //@Override public final void    onDestroy     (             ) {background_service=null; super.onDestroy();}
      @Override public final int     onStartCommand(Intent intent, int flags, int startId)
      {
         Notification notification=getNotification(BACKGROUND_NOTIFICATION_ID, "EE_APP_NAME", background_text, false);
         if(notification!=null)startForeground(BACKGROUND_NOTIFICATION_ID, notification);
         return START_STICKY; // restart if got killed
      }
   }
   public final void startBackgroundService(String text)
   {
      if(background_intent==null)
      {
         background_text=text;
         background_intent=new Intent(this, BackgroundService.class);
         startService(background_intent);
      }
   }
   public final void stopBackgroundService()
   {
      if(background_intent!=null)
      {
         stopService(background_intent);
         background_intent=null;
      }
   }
}
/******************************************************************************/
