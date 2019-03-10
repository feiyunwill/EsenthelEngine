/******************************************************************************/
package EE_PACKAGE;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.NativeActivity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.DialogInterface.OnShowListener;
import android.content.Context;
import android.content.Intent;
import android.content.IntentSender.SendIntentException;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.location.Location;
import android.location.LocationListener;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.provider.Settings.Secure;
import android.text.Editable;
import android.text.InputType;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.view.Gravity;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.EditorInfo;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;
import com.android.vending.billing.IInAppBillingService;
import com.google.android.gms.ads.*;
import com.google.android.vending.expansion.downloader.DownloadProgressInfo;
import com.google.android.vending.expansion.downloader.DownloaderClientMarshaller;
import com.google.android.vending.expansion.downloader.DownloaderServiceMarshaller;
import com.google.android.vending.expansion.downloader.Helpers;
import com.google.android.vending.expansion.downloader.IDownloaderClient;
import com.google.android.vending.expansion.downloader.IDownloaderService;
import com.google.android.vending.expansion.downloader.IStub;
import com.google.android.vending.expansion.downloader.impl.DownloaderService;
import java.io.IOException;
import java.security.InvalidKeyException;
import java.security.KeyFactory;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.PublicKey;
import java.security.Signature;
import java.security.SignatureException;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.X509EncodedKeySpec;
import java.util.ArrayList;
import java.util.List;
/******************************************************************************/
public class LoaderActivity extends Activity implements IDownloaderClient
{
   public static class EsenthelDownloaderService extends DownloaderService // !! if changing name of this class update AndroidManifest.xml !!
   {
      public static final byte[] SALT=new byte[]{104, 43, 93, 85, 109, 101, 44, 18, 18, 19, 55, 22, 108, 29, 63, 64, 5};

      @Override public String getPublicKey() {return "EE_LICENSE_KEY";}
      @Override public byte[] getSALT() {return SALT;}
      @Override public String getAlarmReceiverClassName() {return EsenthelAlarmReceiver.class.getName();}
   }

   public static class EsenthelAlarmReceiver extends BroadcastReceiver // !! if changing name of this class update AndroidManifest.xml !!
   {
      @Override public void onReceive(Context context, Intent intent)
      {
         try
         {
            DownloaderClientMarshaller.startDownloadServiceIfRequired(context, intent, EsenthelDownloaderService.class);
         }catch(NameNotFoundException e) {e.printStackTrace();}
      }
   }

   ProgressBar mProgressBar;
   TextView    mStatusText, mProgressFraction, mProgressPercent, mAverageSpeed, mTimeRemaining;
   View        mDashboard, mCellMessage;
   Button      mPauseButton, mWiFiSettingsButton;
   boolean     mStatePaused;

   IStub              mDownloaderClientStub;
   IDownloaderService mRemoteService;

   private final void setState(int newState)
   {
      mStatusText.setText(Helpers.getDownloaderStringResourceIDFromState(newState));
   }
   private final void setButtonPausedState(boolean paused)
   {
      mStatePaused=paused;
      mPauseButton.setText(paused ? "Resume" : "Pause");
   }
   private final void start()
   {
      Intent intent=new Intent(this, EsenthelActivity.class);
      startActivity(intent);
      finish();
   }

   @Override public void onStart()
   {
      super.onStart();
      if(mDownloaderClientStub!=null)mDownloaderClientStub.connect(this);
   }
   @Override public void onStop()
   {
      super.onStop();
      if(mDownloaderClientStub!=null)mDownloaderClientStub.disconnect(this);
   }
   @Override public void onCreate(Bundle savedInstanceState)
   {
      super.onCreate(savedInstanceState);

      if(!EE_DOWNLOAD_EXPANSION)start();else
      try
      {
         // Build PendingIntent used to open this activity from Notification
         Intent launchIntent=LoaderActivity.this.getIntent();
         Intent notifierIntent=new Intent(LoaderActivity.this, LoaderActivity.this.getClass());
                notifierIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK|Intent.FLAG_ACTIVITY_CLEAR_TOP);
                notifierIntent.setAction(launchIntent.getAction());
         if(launchIntent.getCategories()!=null)
            for(String category:launchIntent.getCategories())
               notifierIntent.addCategory(category);

         PendingIntent pendingIntent=PendingIntent.getActivity(LoaderActivity.this, 0, notifierIntent, PendingIntent.FLAG_UPDATE_CURRENT);

         // Request to start the download
         int startResult =DownloaderClientMarshaller.startDownloadServiceIfRequired(this, pendingIntent, EsenthelDownloaderService.class);
         if( startResult==DownloaderClientMarshaller.NO_DOWNLOAD_REQUIRED)start();else
         {
            mDownloaderClientStub=DownloaderClientMarshaller.CreateStub(this, EsenthelDownloaderService.class);
            setContentView(R.layout.loader);

            mProgressBar=(ProgressBar) findViewById(R.id.progressBar);
            mStatusText=(TextView) findViewById(R.id.statusText);
            mProgressFraction=(TextView) findViewById(R.id.progressAsFraction);
            mProgressPercent=(TextView) findViewById(R.id.progressAsPercentage);
            mAverageSpeed=(TextView) findViewById(R.id.progressAverageSpeed);
            mTimeRemaining=(TextView) findViewById(R.id.progressTimeRemaining);
            mDashboard=findViewById(R.id.downloaderDashboard);
            mCellMessage=findViewById(R.id.approveCellular);
            mPauseButton=(Button) findViewById(R.id.pauseButton);
            mWiFiSettingsButton=(Button) findViewById(R.id.wifiSettingsButton);

            mPauseButton.setOnClickListener(new View.OnClickListener()
            {
               @Override public void onClick(View view)
               {
                  if(mRemoteService!=null)
                  {
                     if(mStatePaused)mRemoteService.requestContinueDownload();
                     else            mRemoteService.requestPauseDownload   ();
                  }
                  setButtonPausedState(!mStatePaused);
               }
            });

            mWiFiSettingsButton.setOnClickListener(new View.OnClickListener()
            {
               @Override public void onClick(View v) {startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS));}
            });

            Button resumeOnCell=(Button) findViewById(R.id.resumeOverCellular);
            resumeOnCell.setOnClickListener(new View.OnClickListener()
            {
               @Override public void onClick(View view)
               {
                  if(mRemoteService!=null)
                  {
                     mRemoteService.setDownloadFlags(IDownloaderService.FLAGS_DOWNLOAD_OVER_CELLULAR);
                     mRemoteService.requestContinueDownload();
                  }
                  mCellMessage.setVisibility(View.GONE);
               }
           });
         }
      }
      catch(NameNotFoundException e) {EsenthelActivity.log("Cannot find own package!"); e.printStackTrace();}
   }
   @Override public void onServiceConnected(Messenger m)
   {
      mRemoteService=DownloaderServiceMarshaller.CreateProxy(m);
      mRemoteService.onClientUpdated(mDownloaderClientStub.getMessenger());
   }
   @Override public void onDownloadStateChanged(int newState)
   {
      setState(newState);
      boolean showDashboard=true;
      boolean showCellMessage=false;
      boolean paused;
      boolean indeterminate;
      switch(newState)
      {
         case IDownloaderClient.STATE_IDLE: // service is listening, so it's safe to start making calls via mRemoteService
            paused=false;
            indeterminate=true;
         break;

         case IDownloaderClient.STATE_CONNECTING:
         case IDownloaderClient.STATE_FETCHING_URL:
            showDashboard=true;
            paused=false;
            indeterminate=true;
         break;

         case IDownloaderClient.STATE_DOWNLOADING:
            paused=false;
            showDashboard=true;
            indeterminate=false;
         break;

         case IDownloaderClient.STATE_FAILED_CANCELED:
         case IDownloaderClient.STATE_FAILED:
         case IDownloaderClient.STATE_FAILED_FETCHING_URL:
         case IDownloaderClient.STATE_FAILED_UNLICENSED:
            paused=true;
            showDashboard=false;
            indeterminate=false;
         break;

         case IDownloaderClient.STATE_PAUSED_NEED_CELLULAR_PERMISSION:
         case IDownloaderClient.STATE_PAUSED_WIFI_DISABLED_NEED_CELLULAR_PERMISSION:
            showDashboard=false;
            paused=true;
            indeterminate=false;
            showCellMessage=true;
         break;

         case IDownloaderClient.STATE_PAUSED_BY_REQUEST:
            paused=true;
            indeterminate=false;
         break;

         case IDownloaderClient.STATE_PAUSED_ROAMING:
         case IDownloaderClient.STATE_PAUSED_SDCARD_UNAVAILABLE:
            paused=true;
            indeterminate=false;
         break;

         case IDownloaderClient.STATE_COMPLETED:
            showDashboard=false;
            paused=false;
            indeterminate=false;
         break;

         default:
            paused=true;
            indeterminate=true;
            showDashboard=true;
         break;
      }
      mDashboard.setVisibility(showDashboard ? View.VISIBLE : View.GONE);
      mCellMessage.setVisibility(showCellMessage ? View.VISIBLE : View.GONE);
      mProgressBar.setIndeterminate(indeterminate);
      setButtonPausedState(paused);
      if(newState==IDownloaderClient.STATE_COMPLETED)start();
   }
   @Override public void onDownloadProgress(DownloadProgressInfo progress)
   {
      mAverageSpeed.setText(getString(R.string.kilobytes_per_second, Helpers.getSpeedString(progress.mCurrentSpeed)));
      mTimeRemaining.setText(getString(R.string.time_remaining, Helpers.getTimeRemaining(progress.mTimeRemaining)));
      progress.mOverallTotal=progress.mOverallTotal;
      mProgressBar.setMax((int) (progress.mOverallTotal >> 8));
      mProgressBar.setProgress((int) (progress.mOverallProgress >> 8));
      mProgressPercent.setText(Long.toString(progress.mOverallProgress * 100 / progress.mOverallTotal) + "%");
      mProgressFraction.setText(Helpers.getDownloadProgressString(progress.mOverallProgress, progress.mOverallTotal));
   }
}
/******************************************************************************/
