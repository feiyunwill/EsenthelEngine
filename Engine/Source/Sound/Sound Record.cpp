/******************************************************************************

   'SoundRecord' methods on DirectSound always require usage of 'SoundAPILock' locks,
      not because of the API thread-safety, but because of:
      -objects are added/removed to 'SoundRecords' list
      -objects can be processed on the main thread by the user, and on the sound thread by the engine

/******************************************************************************/
#include "stdafx.h"
namespace EE{
#if APPLE
   #if MAC
      ASSERT(SIZE(AudioDeviceID)==MEMBER_SIZE(UID, i[0])); // because we're storing 'AudioDeviceID' in the 'UID.i[0]'
   #endif
   #define kOutputBus 0
   #define kInputBus  1
#endif
/******************************************************************************/
#if WINDOWS_NEW || !APPLE && !ANDROID && (DIRECT_SOUND || OPEN_AL)
Memc<SoundRecord*> SoundRecords;
#endif
/******************************************************************************/
#if WINDOWS_NEW
using namespace Microsoft::WRL;
using namespace Windows::Media::Devices;

struct AudioClientGetter : RuntimeClass<RuntimeClassFlags<ClassicCom>, FtmBase, IActivateAudioInterfaceCompletionHandler>
{
   Bool           finished;
   IAudioClient3 *audio_client;

   virtual HRESULT STDMETHODCALLTYPE ActivateCompleted(IActivateAudioInterfaceAsyncOperation *operation)
   {
      finished=true; return S_OK;
   }
   AudioClientGetter()
   {
      audio_client=null;
      if(App.hwnd()) // can call methods only if app was initialized, otherwise it will crash
      {
         finished=false;
         IActivateAudioInterfaceAsyncOperation *audio_activate;
         auto device_name=MediaDevice::GetDefaultAudioCaptureId(AudioDeviceRole::Default); // this will be something like "\\?\SWD#MMDEVAPI#{0.0.1.00000000}.{5871ae83-5ebe-46cc-9bce-b486b524e679}#{2eef81be-33fa-4800-9670-1cd474972c3f}"
         if(OK(ActivateAudioInterfaceAsync(device_name->Data(), __uuidof(IAudioClient3), null, this, &audio_activate)))
         {
            if(App.mainThread())App.loopUntil(finished, true);else for(; !finished; )Time.wait(1); // wait because app may have to wait a long time until user agrees to provide permission which would cause full CPU usage
            HRESULT hr; IUnknown *ac=null; audio_activate->GetActivateResult(&hr, &ac); audio_activate->Release(); if(ac)
            {
               ac->QueryInterface(__uuidof(IAudioClient3), (Ptr*)&audio_client);
               ac->Release();
            }
         }
      }
   }
};
struct _SoundRecord
{
   Int                  block;
   IAudioClient3       *audio_client;
   IAudioCaptureClient *audio_capture_client;
   SyncEvent            samples_ready;

 ~_SoundRecord() {del();}
   void           del()
   {
      RELEASE(audio_capture_client);
      RELEASE(audio_client);
   }
  _SoundRecord()
   {
      block=0;
      audio_client=null;
      audio_capture_client=null;
   }
   Bool create(Int bits, Int channels, Int frequency)
   {
      del();
      if(audio_client=AudioClientGetter().audio_client)
      {
         WAVEFORMATEX *wf=null; audio_client->GetMixFormat(&wf); if(wf)
         {
          C Int bytes=bits/8;

            Zero(*wf);
            wf->wFormatTag     =WAVE_FORMAT_PCM;
            wf->nChannels      =channels;
            wf->nSamplesPerSec =frequency;
            wf->wBitsPerSample =bits; 
            wf->nBlockAlign    =channels*bytes;
            wf->nAvgBytesPerSec=wf->nBlockAlign*frequency;

            UInt default_frames, fundamental_frames, min_frames, max_frames, frames;
            if(OK(audio_client->GetSharedModeEnginePeriod(wf, &default_frames, &fundamental_frames, &min_frames, &max_frames)))
            if(OK(audio_client->InitializeSharedAudioStream(AUDCLNT_STREAMFLAGS_EVENTCALLBACK, min_frames, wf, null)))
            if(OK(audio_client->GetBufferSize(&frames)))
            if(OK(audio_client->GetService(__uuidof(IAudioCaptureClient), (Ptr*)&audio_capture_client)))
            if(OK(audio_client->SetEventHandle(samples_ready.handle()))) // this is needed
            if(OK(audio_client->Start()))
            {
               block=wf->nBlockAlign; // get this once initialization finishes, in case it will be different than what was requested
               return true;
            }
         }
      }
      del(); return false;
   }
   void update(SoundRecord &sound_record)
   {
      for(;;)
      {
         BYTE *data; UINT32 frames; DWORD flags; UINT64 pos, pc_pos;
         if(OK(audio_capture_client->GetNextPacketSize(&frames))
         && frames>0
         && OK(audio_capture_client->GetBuffer(&data, &frames, &flags, &pos, &pc_pos)))
         {
            Int size=frames*block;
            if(flags&AUDCLNT_BUFFERFLAGS_SILENT)Zero(data, size);
          //if(flags&AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY){extern Int lost; lost++;}
            sound_record.receivedData(data, size);
            audio_capture_client->ReleaseBuffer(frames);
         }else break;
      }
   }
};
#endif
/******************************************************************************/
SoundRecord::SoundRecord()
{
  _handle=null;
#if WINDOWS_OLD
  _pos=_size=0;
#endif

#if APPLE
  _flags=0;
#elif ANDROID
#elif DIRECT_SOUND
  _dscb=null;
#elif OPEN_AL
  _block=0;
#endif
}
void SoundRecord::del()
{
#if WINDOWS_NEW
   if(_handle)
   {
      SafeSyncLocker locker(SoundAPILock);
      SoundRecords.exclude(this);
      Delete(_handle);
   }
#elif APPLE
   if(_handle)
   {
      OSStatus status=AudioOutputUnitStop(_handle);
      AudioUnitUninitialize(_handle);
      AudioComponentInstanceDispose(_handle);
     _handle=null;
   }
  _flags=0;
#elif ANDROID
   if(_handle)
   {
      JNI jni; if(jni && ActivityClass)
      {
         Str8 signature=S8+"(L"+AndroidPackageName+"/EsenthelActivity$EsenthelAudioRecord;)V"; signature.replace('.', '/');
         if(JMethodID delAudioRecord=jni->GetStaticMethodID(ActivityClass, "delAudioRecord", signature))
            jni->CallStaticVoidMethod(ActivityClass, delAudioRecord, jobject(_handle));
         jni->DeleteGlobalRef(jobject(_handle));
      }
     _handle=null;
   }
#elif DIRECT_SOUND
   if(_handle || _dscb)
   {
      SafeSyncLocker locker(SoundAPILock);
      SoundRecords.exclude(this);
      RELEASE(_dscb  );
      RELEASE(_handle);
   }
#elif OPEN_AL
   if(_handle)
   {
      SafeSyncLocker locker(SoundAPILock);
      SoundRecords.exclude(this);
      alcCaptureStop       (_handle);
      alcCaptureCloseDevice(_handle);
     _handle=null;
   }
  _block=0;
#endif

#if WINDOWS_OLD
  _pos=_size=0;
#endif
}
/******************************************************************************/
#if APPLE
static OSStatus AudioInputProc(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
   SoundRecord &sr=*(SoundRecord*)inRefCon;
   union
   {
      Byte            extra[SIZE(AudioBufferList)+MEMBER_SIZE(AudioBufferList, mBuffers[0])];
      AudioBufferList list;
   }buffer;

   Memt<Byte> temp;

   UInt flags          =sr._flags;
   Bool int_signed     =(flags>>0)&1,
        non_interleaved=(flags>>1)&1,
        want_stereo    =(flags>>2)&1,
         got_stereo    =(flags>>3)&1;
   UInt want_bytes     =(flags>>4)&7,
         got_bytes     =(flags>>7)&7,
         got_channels  = got_stereo+1,  got_block= got_bytes* got_channels,  got_data_size=inNumberFrames* got_block,
        want_channels  =want_stereo+1, want_block=want_bytes*want_channels, want_data_size=inNumberFrames*want_block;
   Bool  same_format   =(want_stereo==got_stereo && want_bytes==got_bytes && !non_interleaved);
   temp.setNum(same_format ? want_data_size : got_data_size+want_data_size);
   if(non_interleaved)
   {
      UInt data_size_2=got_data_size/2;
      buffer.list.mNumberBuffers=2;
      buffer.list.mBuffers[0].mNumberChannels=buffer.list.mBuffers[1].mNumberChannels=1;
      buffer.list.mBuffers[0].mDataByteSize  =buffer.list.mBuffers[1].mDataByteSize  =data_size_2;
      buffer.list.mBuffers[0].mData=temp.data();
      buffer.list.mBuffers[1].mData=temp.data()+data_size_2;
   }else
   {
      buffer.list.mNumberBuffers=1;
      buffer.list.mBuffers[0].mNumberChannels=got_channels;
      buffer.list.mBuffers[0].mDataByteSize  =got_data_size;
      buffer.list.mBuffers[0].mData=temp.data();
   }
   if(AudioUnitRender(sr._handle, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, &buffer.list)==noErr)
   {
      if(same_format)
      {
         sr.receivedData(temp.data(), temp.elms());
      }else
      {
        CPtr  src=temp.data(), src_left, src_right;
         Ptr dest=temp.data()+got_data_size, dest_temp=dest;
         Int src_inc;
                             src_left =src;
         if(non_interleaved){src_right=(Byte*)src+got_data_size/2; src_inc=got_bytes  ;}else
         if(got_stereo     ){src_right=(Byte*)src+got_bytes      ; src_inc=got_bytes*2;}else
                            {src_right=       src                ; src_inc=got_bytes  ;}

         Bool dest_bit16=(want_bytes==2);
         REP(inNumberFrames)
         {
            Int left, right;
            switch(got_bytes)
            {
               case 1: if(int_signed){left=      (*(I8 *)src_left<<8)       ; right=      (*(I8 *)src_right<<8)       ;}
                       else          {left=      (*(U8 *)src_left<<8)-32768 ; right=      (*(U8 *)src_right<<8)-32768 ;} break;
               case 2: if(int_signed){left=       *(I16*)src_left           ; right=       *(I16*)src_right           ;}
                       else          {left=       *(U16*)src_left    -32768 ; right=       *(U16*)src_right    -32768 ;} break;
               case 4:                left=Round( *(Flt*)src_left    *32767); right=Round( *(Flt*)src_right    *32767);  break;
            }
            src_left =(Byte*)src_left +src_inc;
            src_right=(Byte*)src_right+src_inc;
            if(want_stereo)
            {
               if(dest_bit16)
               {
                  ((I16*)dest)[0]=left ;
                  ((I16*)dest)[1]=right;
                  dest=(I16*)dest+2;
               }else
               {
                  ((U8*)dest)[0]=(left >>8)+128;
                  ((U8*)dest)[1]=(right>>8)+128;
                  dest=(U8*)dest+2;
               }
            }else
            {
               Int sample=((left+right)>>1);
               if(dest_bit16)
               {
                  *(I16*)dest=sample; dest=(I16*)dest+1;
               }else
               {
                  *(U8*)dest=(sample>>8)+128; dest=(U8*)dest+1;
               }
            }
         }
         sr.receivedData(dest_temp, want_data_size);
      }
   }
   return noErr;
}
#endif
Bool SoundRecord::create(Device *device, Int bits, Int channels, Int frequency) // this needs to be called after 'DirectSound' playback device
{
 C Int bytes=bits/8, block=channels*bytes;
#if WINDOWS_NEW
   del();
  _SoundRecord sr; if(sr.create(bits, channels, frequency))
   {
      New(_handle); Swap(*_handle, sr);
      SafeSyncLocker locker(SoundAPILock);
      SoundRecords.add(this);
      return true;
   }
#elif APPLE
   del();
   if(channels>=1 && channels<=2 && (bits==8 || bits==16))
   {
      UInt32   size;
      OSStatus status;
   #if MAC
      AudioDeviceID device_id=(device ? device->_id.i[0] : kAudioDeviceUnknown);
      if(device_id==kAudioDeviceUnknown)
      {
         AudioObjectPropertyAddress propertyAddress;
         propertyAddress.mSelector=kAudioHardwarePropertyDefaultInputDevice;
         propertyAddress.mScope=kAudioObjectPropertyScopeGlobal;
         propertyAddress.mElement=kAudioObjectPropertyElementMaster;

         size=SIZE(device_id); status=AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, null, &size, &device_id);
      }
      if(device_id!=kAudioDeviceUnknown)
   #elif IOS
      [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord error:nil];
      [[AVAudioSession sharedInstance] setActive:YES error:nil];
   #endif
      {
         AudioComponentDescription desc;
         desc.componentType=kAudioUnitType_Output;
      #if MAC
	      desc.componentSubType=kAudioUnitSubType_HALOutput;
      #elif IOS
	      desc.componentSubType=kAudioUnitSubType_RemoteIO;
      #else
         #error unknown platform
      #endif
	      desc.componentManufacturer=kAudioUnitManufacturer_Apple;
	      desc.componentFlags=0;
	      desc.componentFlagsMask=0;
         if(AudioComponent inputComponent=AudioComponentFindNext(null, &desc))
         {
            AudioComponentInstanceNew(inputComponent, &_handle);
            if(_handle)
            {
               UInt32 zero=0, one=1;
            #if MAC
	        status=AudioUnitSetProperty(_handle, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, kOutputBus, &zero, SIZE(zero));        // disable output
            #endif
	            if(AudioUnitSetProperty(_handle, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input , kInputBus , &one , SIZE(one ))==noErr) // enable  input
            #if MAC
               if(AudioUnitSetProperty(_handle, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, kOutputBus, &device_id, SIZE(device_id))==noErr) // set device
            #endif
               {
                  AudioStreamBasicDescription format;
                  format.mSampleRate=frequency;
                  format.mFormatID=kAudioFormatLinearPCM;
                  format.mFormatFlags=((bits==16) ? kAudioFormatFlagIsSignedInteger : 0)|kAudioFormatFlagIsPacked;
                  format.mBytesPerPacket=bytes*channels;
                  format.mFramesPerPacket=1;
                  format.mBytesPerFrame=bytes*channels;
                  format.mChannelsPerFrame=channels;
                  format.mBitsPerChannel=bits;
                  format.mReserved=0;

	               // set desired format
                  status=AudioUnitSetProperty(_handle, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, kInputBus, &format, SIZE(format)); //LogN(S+"set:"+Int(status));

                  // get actual format
                  size=SIZE(format);
	               status=AudioUnitGetProperty(_handle, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, kInputBus, &format, &size); //LogN(S+"get:"+Int(status)+", frequency:"+Round(format.mSampleRate)+" flags:"+UInt(format.mFormatFlags)+" bits:"+Int(format.mBitsPerChannel)+" ch:"+Int(format.mChannelsPerFrame)+" bytespp:"+Int(format.mBytesPerPacket)+" bytespf:"+Int(format.mBytesPerFrame)+" framespp:"+Int(format.mFramesPerPacket));
                  UInt got_channels=format.mChannelsPerFrame,
                       got_bits    =format.mBitsPerChannel;
                  if(status==noErr && got_channels>=1 && got_channels<=2 && (got_bits==8 || got_bits==16 || got_bits==32))
                  {
                     //UInt32 samples=0; size=SIZE(samples); status=AudioUnitGetProperty(_handle, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &samples, &size);

                    _flags=(
                        ((                   FlagTest(format.mFormatFlags, kAudioFormatFlagIsSignedInteger ))<<0)
                     |  ((got_channels==2 && FlagTest(format.mFormatFlags, kAudioFormatFlagIsNonInterleaved))<<1)
                     |  ((    channels==2)<<2)
                     |  ((got_channels==2)<<3)
                     |  ((    bits    / 8)<<4)
                     |  ((got_bits    / 8)<<7));

                     AURenderCallbackStruct callback;
                     callback.inputProc=AudioInputProc;
	                  callback.inputProcRefCon=this;
	                  if(AudioUnitSetProperty(_handle, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 0, &callback, SIZE(callback))==noErr)
	                  if(AudioUnitInitialize(_handle)==noErr)
                     if(AudioOutputUnitStart(_handle)==noErr)
                        return true;
                  }
               }
            }
         }
      }
   }
#elif ANDROID
   del();
   JNI jni; 
   if( jni && ActivityClass)
   {
      RequirePermission(PERMISSION_SOUND_RECORD);

      Str8 signature=S8+"(JIII)L"+AndroidPackageName+"/EsenthelActivity$EsenthelAudioRecord;"; signature.replace('.', '/');
      if(JMethodID newAudioRecord=jni->GetStaticMethodID(ActivityClass, "newAudioRecord", signature))
         if(JObject audio_record=JObject(jni, jni->CallStaticObjectMethod(ActivityClass, newAudioRecord, jlong(this), jint(bits), jint(channels), jint(frequency))))
      {
         audio_record.makeGlobal(); _handle=audio_record(); audio_record.clear();
         return true;
      }
   }
#elif DIRECT_SOUND
   SafeSyncLocker locker(SoundAPILock);
   del();
   if(channels>=1 && channels<=2 && (bits==8 || bits==16))
   {
      DirectSoundCaptureCreate8(device ? &device->_id.guid() : null, &_handle, null);
      if(_handle)
      {
         WAVEFORMATEX wfx; Zero(wfx);
         wfx.wFormatTag     =WAVE_FORMAT_PCM;
         wfx.nChannels      =channels;
         wfx.nSamplesPerSec =frequency;
         wfx.wBitsPerSample =bits; 
         wfx.nBlockAlign    =block;
         wfx.nAvgBytesPerSec=block*frequency;

         DSCBUFFERDESC desc; Zero(desc);
         desc.dwSize=SIZE(desc);
         desc.dwFlags=DSCBCAPS_WAVEMAPPED; // 'DSCBCAPS_WAVEMAPPED'=The Win32 wave mapper will be used for formats not supported by the device
         desc.lpwfxFormat=&wfx;
         desc.dwBufferBytes=(frequency*SOUND_TIME_RECORD/1000)*block; // mul by 'block' last to make sure that size is aligned to it
        _handle->CreateCaptureBuffer(&desc, &_dscb, null);
         if(_dscb)
         {
           _size=desc.dwBufferBytes;
           _dscb->Start(DSCBSTART_LOOPING);
          //Zero(wfx); _dscb->GetFormat(&wfx, SIZE(wfx), null);
            SoundRecords.add(this);
            return true;
         }
      }
   }
#elif OPEN_AL
   SafeSyncLocker locker(SoundAPILock);
   del();
   if(channels>=1 && channels<=2 && (bits==8 || bits==16))
   {
      if(_handle=alcCaptureOpenDevice(device ? Str8(device->name)() : null, frequency,
         (channels==1) ? ((bits==8) ? AL_FORMAT_MONO8   : AL_FORMAT_MONO16  )
                       : ((bits==8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16), frequency*SOUND_TIME_RECORD/1000))
      {
         alcCaptureStart(_handle);
        _block=block;
         SoundRecords.add(this);
         return true;
      }
   }
#endif
   del(); return false;
}
/******************************************************************************/
#if DIRECT_SOUND
static BOOL CALLBACK SoundRecordDeviceCallback(LPGUID lpGuid, LPCWSTR lpcstrDescription, LPCWSTR lpcstrModule, LPVOID lpContext)
{
   MemPtr<SoundRecord::Device> &devices=*(MemPtr<SoundRecord::Device>*)lpContext;
          SoundRecord::Device  &device =devices.New();
   device.name=lpcstrDescription;
   if(lpGuid)device._id.guid()=*lpGuid;
   return true; // keep going
}
#endif
void SoundRecord::GetDevices(MemPtr<Device> devices)
{
   devices.clear();
#if WINDOWS_NEW
   if(auto id=MediaDevice::GetDefaultAudioCaptureId(AudioDeviceRole::Default))if(Is(id->Data()))
      devices.New().name="Microphone";
#elif MAC
   AudioObjectPropertyAddress propertyAddress=
   { 
      kAudioHardwarePropertyDevices,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMaster,
   };

   UInt32 dataSize=0; if(AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, null, &dataSize)==kAudioHardwareNoError)
   {
      MemtN<AudioDeviceID, 16> audioDevices; audioDevices.setNum(dataSize/SIZE(AudioDeviceID));
      if(audioDevices.elms())
      {
         dataSize=audioDevices.elms()*SIZE(AudioDeviceID);
         if(AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, null, &dataSize, audioDevices.data())==kAudioHardwareNoError)
         {
            // Iterate through all the devices and determine which are input-capable
            propertyAddress.mScope=kAudioDevicePropertyScopeInput;
            FREPA(audioDevices)
            {
               // check if this is an input device
               propertyAddress.mSelector=kAudioDevicePropertyStreams;
               AudioObjectGetPropertyDataSize(audioDevices[i], &propertyAddress, 0, null, &dataSize);
               UInt streamCount=dataSize/SIZE(AudioStreamID);
               if(  streamCount>0) 
               {
                  Device &device=devices.New();
                  device._id.i[0]=audioDevices[i];
                  
                  // Query device name
                  CFStringRef deviceName=null; dataSize=SIZE(deviceName);
                  propertyAddress.mSelector=kAudioDevicePropertyDeviceNameCFString;
                  if(AudioObjectGetPropertyData(audioDevices[i], &propertyAddress, 0, null, &dataSize, &deviceName)==kAudioHardwareNoError) {}
                  if(deviceName)
                  {
                     Char8 str[1024]; str[0]='\0'; CFStringGetCString(deviceName, str, Elms(str), kCFStringEncodingUTF8);
                     device.name=FromUTF8(str);
                     CFRelease(deviceName);
                  }

                  /*
                  // Query device UID
                  CFStringRef deviceUID=null; dataSize=SIZE(deviceUID);
                  propertyAddress.mSelector=kAudioDevicePropertyDeviceUID;
                  if(AudioObjectGetPropertyData(audioDevices[i], &propertyAddress, 0, null, &dataSize, &deviceUID)==kAudioHardwareNoError) {}
                  if(deviceUID)CFRelease(deviceUID);

                  // Query device manufacturer
                  CFStringRef deviceManufacturer=null; dataSize=SIZE(deviceManufacturer);
                  propertyAddress.mSelector=kAudioDevicePropertyDeviceManufacturerCFString;
                  if(AudioObjectGetPropertyData(audioDevices[i], &propertyAddress, 0, null, &dataSize, &deviceManufacturer)==kAudioHardwareNoError) {}
                  if(deviceManufacturer)CFRelease(deviceManufacturer);

                  // Determine if the device is an input device (it is an input device if it has input channels)
                  dataSize=0;
                  propertyAddress.mSelector=kAudioDevicePropertyStreamConfiguration;
                  if(AudioObjectGetPropertyDataSize(audioDevices[i], &propertyAddress, 0, null, &dataSize)==kAudioHardwareNoError)
                  {
                     MemtN<AudioBufferList, 16> bufferList; bufferList.setNum(dataSize/SIZE(AudioBufferList));
                   //LogN(S+"bufferList:"+bufferList.elms());
                     if(bufferList.elms())
                     {
                        dataSize=bufferList.elms()*SIZE(AudioBufferList);
                        if(AudioObjectGetPropertyData(audioDevices[i], &propertyAddress, 0, null, &dataSize, bufferList.data())==kAudioHardwareNoError)
                        {
                         //LogN(S+"bufferList[0].mNumberBuffers:"+bufferList[0].mNumberBuffers);
                           if(bufferList[0].mNumberBuffers)
                           {
                     
                           }
                        }
                     }
                  }*/
               }
            }
         }
      }
   }
#elif IOS
   if([AVAudioSession sharedInstance].inputAvailable)devices.New().name="Microphone";
#elif ANDROID
   JNI jni; 
   if( jni && ActivityClass)
      if(JMethodID hasAudioRecord=jni->GetStaticMethodID(ActivityClass, "hasAudioRecord", "()Z"))
         if(jni->CallStaticBooleanMethod(ActivityClass, hasAudioRecord))
            devices.New().name="Microphone";
#elif DIRECT_SOUND
   DirectSoundCaptureEnumerate(SoundRecordDeviceCallback, &devices);
#elif OPEN_AL
   if(CChar8 *s=alcGetString(null, ALC_CAPTURE_DEVICE_SPECIFIER))
      for(; *s; s+=Length(s)+1)
         devices.New().name=s;
#endif
}
/******************************************************************************/
Int SoundRecord::curPosNoLock()C
{
#if DIRECT_SOUND
   DWORD capture=0, read=0; // The capture cursor is ahead of the read cursor. The data after the read position up to and including the capture position is not necessarily valid data.
   if(_dscb)_dscb->GetCurrentPosition(&capture, &read);
   return read;
#else
   return 0;
#endif
}
void SoundRecord::updateNoLock()
{
#if WINDOWS_NEW
   if(_handle)_handle->update(T);
#elif APPLE
#elif ANDROID
#elif DIRECT_SOUND
   if(_dscb)
   {
      Int cur_pos =curPosNoLock();
      if( cur_pos!=_pos)
      {
         Int size=cur_pos-_pos; if(size<0)size+=_size;
         Ptr data=null, data2=null; DWORD locked=0, locked2=0;
         if(OK(_dscb->Lock(_pos, size, &data, &locked, &data2, &locked2, 0)))
         {
            if(data )receivedData(data , locked );
            if(data2)receivedData(data2, locked2);
           _dscb->Unlock(data, locked, data2, locked2);
         }
        _pos=cur_pos;
      }
   }
#elif OPEN_AL
   if(_handle)
   {
   again:
      ALint samples=0; alcGetIntegerv(_handle, ALC_CAPTURE_SAMPLES, 1, &samples);
      if(   samples>0)
      {
         Byte data[64*1024]; Int size=samples*_block; Bool full=(size>=SIZE(data)); if(full)
         {
            samples=SIZEU(data)/_block;
            size   =SIZE (data);
         }
         alcCaptureSamples(_handle, data, samples);
         receivedData(data, size);
         if(full)goto again;
      }
   }
#endif
}
void SoundRecord::update()
{
   SafeSyncLocker locker(SoundAPILock);
   updateNoLock();
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
#if ANDROID
extern "C"
{

JNIEXPORT void JNICALL Java_com_esenthel_Native_audioRecord(JNIEnv *env, jclass clazz, jlong sound_record, jobject buf, jint size)
{
   if(Ptr data=env->GetDirectBufferAddress(buf))
   {
      SoundRecord *sr=(SoundRecord*)sound_record;
      sr->receivedData(data, size);
   }
}

}
#endif
/******************************************************************************/
