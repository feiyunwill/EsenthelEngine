/******************************************************************************/
#if EE_PRIVATE
   #if ANDROID
/******************************************************************************/
enum KEY_SOURCE
{
   KEY_ANY ,
   KEY_CPP ,
   KEY_JAVA,
};
/******************************************************************************/
struct JNI
{
           operator JNIEnv*()C {return _;}
   JNIEnv* operator->      ()C {return _;}
   Str     operator()      (jstring str)C;

   void clear ();
   void del   ();
   void attach();

   explicit JNI(JNIEnv *jni) {clear(); _=jni;}
            JNI(           ) {clear(); attach();}
           ~JNI(           ) {del();}

private:
   JNIEnv *_;
   Bool    attached;
   NO_COPY_CONSTRUCTOR(JNI);
}extern
   Jni;

struct JObject
{
   operator Bool      ()C {return _!=null;}
   operator jobject   ()C {return _;}
   jobject  operator()()C {return _;}

   JObject& clear     (); // clear without deleting
   JObject& del       ();
   JObject& makeGlobal();
   JObject& operator= (jobject j);

           ~JObject() {del();}
            JObject(          jobject j=null) : _jni(Jni), _(j), _global(false) {}
   explicit JObject(JNI &jni, jobject j=null) : _jni(jni), _(j), _global(false) {}
            JObject(JObject &&temp) : _jni(temp._jni), _(temp._), _global(temp._global) {temp.clear();}

protected:
   jobject _;
   JNI    &_jni;
   Bool    _global;

   NO_COPY_CONSTRUCTOR(JObject);
};

struct JClass : JObject
{
   operator jclass()C {return (jclass)T();}

   JClass& operator=(CChar8 *name);
   JClass& operator=(jobject obj );
   JClass& operator=(jclass  cls );

            JClass(          CChar8 *name=null);
            JClass(          jobject obj      );
            JClass(          jclass  cls      );
   explicit JClass(JNI &jni, CChar8 *name=null);
   explicit JClass(JNI &jni, jobject obj      );
   explicit JClass(JNI &jni, jclass  cls      );
            JClass(JClass &&temp) : JObject(temp._jni, temp._) {_global=temp._global; temp.clear();}
};

struct JString : JObject
{
   operator jstring()C {return (jstring)T();}

   Str str()C {return _jni(T);}

   JString& operator=(jobject j) {JObject::operator=(j); return T;}
   JString& operator=(CChar8 *t);
   JString& operator=(CChar  *t);
   JString& operator=(C Str8 &s);
   JString& operator=(C Str  &s);

            JString(          jobject j=null) : JObject(     j   ) {}
            JString(          CChar8 *t     ) : JObject(     null) {T=t;}
            JString(          CChar  *t     ) : JObject(     null) {T=t;}
            JString(          C Str8 &s     ) : JObject(     null) {T=s;}
            JString(          C Str  &s     ) : JObject(     null) {T=s;}
   explicit JString(JNI &jni, jobject j=null) : JObject(jni, j   ) {}
   explicit JString(JNI &jni, CChar8 *t     ) : JObject(jni, null) {T=t;}
   explicit JString(JNI &jni, CChar  *t     ) : JObject(jni, null) {T=t;}
   explicit JString(JNI &jni, C Str8 &s     ) : JObject(jni, null) {T=s;}
   explicit JString(JNI &jni, C Str  &s     ) : JObject(jni, null) {T=s;}
            JString(JString &&temp) : JObject(temp._jni, temp._) {_global=temp._global; temp.clear();}
};

struct JObjectArray : JObject
{
   operator jobjectArray()C {return (jobjectArray)T();}

   Int     elms      (     )C;
   jobject operator[](Int i)C;

   JObjectArray& operator=(jobject j) {JObject::operator=(j); return T;}

   void set(Int i, CChar8 *t); // set i-th array element as 't' text

            JObjectArray(          jobject j=null) : JObject(     j) {}
   explicit JObjectArray(JNI &jni, jobject j=null) : JObject(jni, j) {}
   explicit JObjectArray(JNI &jni, int     elms  );
            JObjectArray(JObjectArray &&temp) : JObject(temp._jni, temp._) {_global=temp._global; temp.clear();}
};

typedef jmethodID JMethodID;
typedef jfieldID  JFieldID;
/******************************************************************************/
extern jobject   Activity; // 'Activity' should not use 'JObject' because it's not dynamically obtained
extern JClass    ActivityClass, ClipboardManagerClass;
extern JObject   ClipboardManager, LocationManager, GPS_PROVIDER, NETWORK_PROVIDER, EsenthelLocationListener[2];
extern JMethodID getLastKnownLocation, getLatitude, getLongitude, getAltitude, getAccuracy, getSpeed, getTime, requestLocationUpdates, removeUpdates;

extern android_app   *AndroidApp;
extern Str8           AndroidPackageName;
extern Str            AndroidAppPath, AndroidAppDataPath, AndroidAppDataPublicPath, AndroidPublicPath, AndroidSDCardPath;
extern Byte           KeySource;
extern Int            AndroidSDK;
extern AAssetManager *AssetManager;
/******************************************************************************/
   #endif
#endif
/******************************************************************************/
