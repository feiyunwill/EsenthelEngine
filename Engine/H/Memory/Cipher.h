/******************************************************************************/
struct Cipher // Memory De/Encryption Interface
{
   virtual void encrypt(Ptr dest, CPtr src, IntPtr size, Int offset)=NULL; // encrypt 'src' data into 'dest' of 'size' bytes, 'offset'=offset of source data in the stream, if overriding this method, then it should handle the case when "src==null" but "size>0", in this case 'src' should be treated as zeroed memory of 'size' length
   virtual void decrypt(Ptr dest, CPtr src, IntPtr size, Int offset)=NULL; // decrypt 'src' data into 'dest' of 'size' bytes, 'offset'=offset of source data in the stream, if overriding this method, then it should handle the case when "src==null" but "size>0", in this case 'src' should be treated as zeroed memory of 'size' length

   virtual void randomizeKey(       )  {             } // randomize key
   virtual Bool      saveKey(File &f)C {return false;} // save key into 'f' file, return false on fail
   virtual Bool      loadKey(File &f)  {return false;} // load key from 'f' file, return false on fail
   virtual Bool       mixKey(File &f)  {return false;} // mix current key with the key stored in 'f' file, operation should be symmetrical (it should provide the same results as if the keys were swapped - key from file was mixed with current key), return false on fail
};
/******************************************************************************/
struct Cipher1 : Cipher
{
   Cipher1& setKey(C Byte *key, Int key_size); // set key from 'key' array of 'key_size' bytes ('key_size' can be any size, but only up to first 256 bytes will be used)
#if EE_PRIVATE
   void setDec();
   void wrapKey();
#endif

   virtual void encrypt(Ptr dest, CPtr src, IntPtr size, Int offset)override; // encrypt 'src' data into 'dest' of 'size' bytes, 'offset'=offset of source data in the stream
   virtual void decrypt(Ptr dest, CPtr src, IntPtr size, Int offset)override; // decrypt 'src' data into 'dest' of 'size' bytes, 'offset'=offset of source data in the stream

   virtual void randomizeKey(       )  override;
   virtual Bool      saveKey(File &f)C override; // save key into 'f' file, return false on fail
   virtual Bool      loadKey(File &f)  override; // load key from 'f' file, return false on fail
   virtual Bool       mixKey(File &f)  override; // mix current key with the key stored in 'f' file, operation should be symmetrical (it should provide the same results as if the keys were swapped - key from file was mixed with current key), return false on fail

   Cipher1();
   Cipher1(C Byte *key, Int key_size) {setKey(key, key_size);}
   Cipher1(Byte k00, Byte k01, Byte k02, Byte k03, Byte k04, Byte k05, Byte k06, Byte k07, Byte k08, Byte k09, Byte k0A, Byte k0B, Byte k0C, Byte k0D, Byte k0E, Byte k0F,
           Byte k10, Byte k11, Byte k12, Byte k13, Byte k14, Byte k15, Byte k16, Byte k17, Byte k18, Byte k19, Byte k1A, Byte k1B, Byte k1C, Byte k1D, Byte k1E, Byte k1F,
           Byte k20, Byte k21, Byte k22, Byte k23, Byte k24, Byte k25, Byte k26, Byte k27, Byte k28, Byte k29, Byte k2A, Byte k2B, Byte k2C, Byte k2D, Byte k2E, Byte k2F,
           Byte k30, Byte k31, Byte k32, Byte k33, Byte k34, Byte k35, Byte k36, Byte k37, Byte k38, Byte k39, Byte k3A, Byte k3B, Byte k3C, Byte k3D, Byte k3E, Byte k3F,
           Byte k40, Byte k41, Byte k42, Byte k43, Byte k44, Byte k45, Byte k46, Byte k47, Byte k48, Byte k49, Byte k4A, Byte k4B, Byte k4C, Byte k4D, Byte k4E, Byte k4F,
           Byte k50, Byte k51, Byte k52, Byte k53, Byte k54, Byte k55, Byte k56, Byte k57, Byte k58, Byte k59, Byte k5A, Byte k5B, Byte k5C, Byte k5D, Byte k5E, Byte k5F,
           Byte k60, Byte k61, Byte k62, Byte k63, Byte k64, Byte k65, Byte k66, Byte k67, Byte k68, Byte k69, Byte k6A, Byte k6B, Byte k6C, Byte k6D, Byte k6E, Byte k6F,
           Byte k70, Byte k71, Byte k72, Byte k73, Byte k74, Byte k75, Byte k76, Byte k77, Byte k78, Byte k79, Byte k7A, Byte k7B, Byte k7C, Byte k7D, Byte k7E, Byte k7F,
           Byte k80, Byte k81, Byte k82, Byte k83, Byte k84, Byte k85, Byte k86, Byte k87, Byte k88, Byte k89, Byte k8A, Byte k8B, Byte k8C, Byte k8D, Byte k8E, Byte k8F,
           Byte k90, Byte k91, Byte k92, Byte k93, Byte k94, Byte k95, Byte k96, Byte k97, Byte k98, Byte k99, Byte k9A, Byte k9B, Byte k9C, Byte k9D, Byte k9E, Byte k9F,
           Byte kA0, Byte kA1, Byte kA2, Byte kA3, Byte kA4, Byte kA5, Byte kA6, Byte kA7, Byte kA8, Byte kA9, Byte kAA, Byte kAB, Byte kAC, Byte kAD, Byte kAE, Byte kAF,
           Byte kB0, Byte kB1, Byte kB2, Byte kB3, Byte kB4, Byte kB5, Byte kB6, Byte kB7, Byte kB8, Byte kB9, Byte kBA, Byte kBB, Byte kBC, Byte kBD, Byte kBE, Byte kBF,
           Byte kC0, Byte kC1, Byte kC2, Byte kC3, Byte kC4, Byte kC5, Byte kC6, Byte kC7, Byte kC8, Byte kC9, Byte kCA, Byte kCB, Byte kCC, Byte kCD, Byte kCE, Byte kCF,
           Byte kD0, Byte kD1, Byte kD2, Byte kD3, Byte kD4, Byte kD5, Byte kD6, Byte kD7, Byte kD8, Byte kD9, Byte kDA, Byte kDB, Byte kDC, Byte kDD, Byte kDE, Byte kDF,
           Byte kE0, Byte kE1, Byte kE2, Byte kE3, Byte kE4, Byte kE5, Byte kE6, Byte kE7, Byte kE8, Byte kE9, Byte kEA, Byte kEB, Byte kEC, Byte kED, Byte kEE, Byte kEF,
           Byte kF0, Byte kF1, Byte kF2, Byte kF3, Byte kF4, Byte kF5, Byte kF6, Byte kF7, Byte kF8, Byte kF9, Byte kFA, Byte kFB, Byte kFC, Byte kFD, Byte kFE, Byte kFF);

private:
   Byte _enc[256+4], _dec[256+4];
};
/******************************************************************************/
struct Cipher2 : Cipher
{
   Cipher2& setKey(C Byte *key, Int key_size); // set key from 'key' array of 'key_size' bytes ('key_size' can be any size, but only up to first 256 bytes will be used)
#if EE_PRIVATE
   void setDec();
#endif

   virtual void encrypt(Ptr dest, CPtr src, IntPtr size, Int offset)override; // encrypt 'src' data into 'dest' of 'size' bytes, 'offset'=offset of source data in the stream
   virtual void decrypt(Ptr dest, CPtr src, IntPtr size, Int offset)override; // decrypt 'src' data into 'dest' of 'size' bytes, 'offset'=offset of source data in the stream

   virtual void randomizeKey(       )  override;
   virtual Bool      saveKey(File &f)C override; // save key into 'f' file, return false on fail
   virtual Bool      loadKey(File &f)  override; // load key from 'f' file, return false on fail
   virtual Bool       mixKey(File &f)  override; // mix current key with the key stored in 'f' file, operation should be symmetrical (it should provide the same results as if the keys were swapped - key from file was mixed with current key), return false on fail

   Cipher2();
   Cipher2(C Byte *key, Int key_size) {setKey(key, key_size);}
   Cipher2(Byte k00, Byte k01, Byte k02, Byte k03, Byte k04, Byte k05, Byte k06, Byte k07, Byte k08, Byte k09, Byte k0A, Byte k0B, Byte k0C, Byte k0D, Byte k0E, Byte k0F,
           Byte k10, Byte k11, Byte k12, Byte k13, Byte k14, Byte k15, Byte k16, Byte k17, Byte k18, Byte k19, Byte k1A, Byte k1B, Byte k1C, Byte k1D, Byte k1E, Byte k1F,
           Byte k20, Byte k21, Byte k22, Byte k23, Byte k24, Byte k25, Byte k26, Byte k27, Byte k28, Byte k29, Byte k2A, Byte k2B, Byte k2C, Byte k2D, Byte k2E, Byte k2F,
           Byte k30, Byte k31, Byte k32, Byte k33, Byte k34, Byte k35, Byte k36, Byte k37, Byte k38, Byte k39, Byte k3A, Byte k3B, Byte k3C, Byte k3D, Byte k3E, Byte k3F,
           Byte k40, Byte k41, Byte k42, Byte k43, Byte k44, Byte k45, Byte k46, Byte k47, Byte k48, Byte k49, Byte k4A, Byte k4B, Byte k4C, Byte k4D, Byte k4E, Byte k4F,
           Byte k50, Byte k51, Byte k52, Byte k53, Byte k54, Byte k55, Byte k56, Byte k57, Byte k58, Byte k59, Byte k5A, Byte k5B, Byte k5C, Byte k5D, Byte k5E, Byte k5F,
           Byte k60, Byte k61, Byte k62, Byte k63, Byte k64, Byte k65, Byte k66, Byte k67, Byte k68, Byte k69, Byte k6A, Byte k6B, Byte k6C, Byte k6D, Byte k6E, Byte k6F,
           Byte k70, Byte k71, Byte k72, Byte k73, Byte k74, Byte k75, Byte k76, Byte k77, Byte k78, Byte k79, Byte k7A, Byte k7B, Byte k7C, Byte k7D, Byte k7E, Byte k7F,
           Byte k80, Byte k81, Byte k82, Byte k83, Byte k84, Byte k85, Byte k86, Byte k87, Byte k88, Byte k89, Byte k8A, Byte k8B, Byte k8C, Byte k8D, Byte k8E, Byte k8F,
           Byte k90, Byte k91, Byte k92, Byte k93, Byte k94, Byte k95, Byte k96, Byte k97, Byte k98, Byte k99, Byte k9A, Byte k9B, Byte k9C, Byte k9D, Byte k9E, Byte k9F,
           Byte kA0, Byte kA1, Byte kA2, Byte kA3, Byte kA4, Byte kA5, Byte kA6, Byte kA7, Byte kA8, Byte kA9, Byte kAA, Byte kAB, Byte kAC, Byte kAD, Byte kAE, Byte kAF,
           Byte kB0, Byte kB1, Byte kB2, Byte kB3, Byte kB4, Byte kB5, Byte kB6, Byte kB7, Byte kB8, Byte kB9, Byte kBA, Byte kBB, Byte kBC, Byte kBD, Byte kBE, Byte kBF,
           Byte kC0, Byte kC1, Byte kC2, Byte kC3, Byte kC4, Byte kC5, Byte kC6, Byte kC7, Byte kC8, Byte kC9, Byte kCA, Byte kCB, Byte kCC, Byte kCD, Byte kCE, Byte kCF,
           Byte kD0, Byte kD1, Byte kD2, Byte kD3, Byte kD4, Byte kD5, Byte kD6, Byte kD7, Byte kD8, Byte kD9, Byte kDA, Byte kDB, Byte kDC, Byte kDD, Byte kDE, Byte kDF,
           Byte kE0, Byte kE1, Byte kE2, Byte kE3, Byte kE4, Byte kE5, Byte kE6, Byte kE7, Byte kE8, Byte kE9, Byte kEA, Byte kEB, Byte kEC, Byte kED, Byte kEE, Byte kEF,
           Byte kF0, Byte kF1, Byte kF2, Byte kF3, Byte kF4, Byte kF5, Byte kF6, Byte kF7, Byte kF8, Byte kF9, Byte kFA, Byte kFB, Byte kFC, Byte kFD, Byte kFE, Byte kFF);

private:
   Byte _enc[256], _dec[256];
};
/******************************************************************************/
struct Cipher3 : Cipher
{
   Cipher3& setKey(C Byte *key, Int key_size); // set key from 'key' array of 'key_size' bytes ('key_size' can be any size, but only up to first 256 bytes will be used)
#if EE_PRIVATE
   void setDec();
#endif

   virtual void encrypt(Ptr dest, CPtr src, IntPtr size, Int offset)override; // encrypt 'src' data into 'dest' of 'size' bytes, 'offset'=offset of source data in the stream
   virtual void decrypt(Ptr dest, CPtr src, IntPtr size, Int offset)override; // decrypt 'src' data into 'dest' of 'size' bytes, 'offset'=offset of source data in the stream

   virtual void randomizeKey(       )  override;
   virtual Bool      saveKey(File &f)C override; // save key into 'f' file, return false on fail
   virtual Bool      loadKey(File &f)  override; // load key from 'f' file, return false on fail
   virtual Bool       mixKey(File &f)  override; // mix current key with the key stored in 'f' file, operation should be symmetrical (it should provide the same results as if the keys were swapped - key from file was mixed with current key), return false on fail

   Cipher3();
   Cipher3(C Byte *key, Int key_size) {setKey(key, key_size);}
   Cipher3(Byte k00, Byte k01, Byte k02, Byte k03, Byte k04, Byte k05, Byte k06, Byte k07, Byte k08, Byte k09, Byte k0A, Byte k0B, Byte k0C, Byte k0D, Byte k0E, Byte k0F,
           Byte k10, Byte k11, Byte k12, Byte k13, Byte k14, Byte k15, Byte k16, Byte k17, Byte k18, Byte k19, Byte k1A, Byte k1B, Byte k1C, Byte k1D, Byte k1E, Byte k1F,
           Byte k20, Byte k21, Byte k22, Byte k23, Byte k24, Byte k25, Byte k26, Byte k27, Byte k28, Byte k29, Byte k2A, Byte k2B, Byte k2C, Byte k2D, Byte k2E, Byte k2F,
           Byte k30, Byte k31, Byte k32, Byte k33, Byte k34, Byte k35, Byte k36, Byte k37, Byte k38, Byte k39, Byte k3A, Byte k3B, Byte k3C, Byte k3D, Byte k3E, Byte k3F,
           Byte k40, Byte k41, Byte k42, Byte k43, Byte k44, Byte k45, Byte k46, Byte k47, Byte k48, Byte k49, Byte k4A, Byte k4B, Byte k4C, Byte k4D, Byte k4E, Byte k4F,
           Byte k50, Byte k51, Byte k52, Byte k53, Byte k54, Byte k55, Byte k56, Byte k57, Byte k58, Byte k59, Byte k5A, Byte k5B, Byte k5C, Byte k5D, Byte k5E, Byte k5F,
           Byte k60, Byte k61, Byte k62, Byte k63, Byte k64, Byte k65, Byte k66, Byte k67, Byte k68, Byte k69, Byte k6A, Byte k6B, Byte k6C, Byte k6D, Byte k6E, Byte k6F,
           Byte k70, Byte k71, Byte k72, Byte k73, Byte k74, Byte k75, Byte k76, Byte k77, Byte k78, Byte k79, Byte k7A, Byte k7B, Byte k7C, Byte k7D, Byte k7E, Byte k7F,
           Byte k80, Byte k81, Byte k82, Byte k83, Byte k84, Byte k85, Byte k86, Byte k87, Byte k88, Byte k89, Byte k8A, Byte k8B, Byte k8C, Byte k8D, Byte k8E, Byte k8F,
           Byte k90, Byte k91, Byte k92, Byte k93, Byte k94, Byte k95, Byte k96, Byte k97, Byte k98, Byte k99, Byte k9A, Byte k9B, Byte k9C, Byte k9D, Byte k9E, Byte k9F,
           Byte kA0, Byte kA1, Byte kA2, Byte kA3, Byte kA4, Byte kA5, Byte kA6, Byte kA7, Byte kA8, Byte kA9, Byte kAA, Byte kAB, Byte kAC, Byte kAD, Byte kAE, Byte kAF,
           Byte kB0, Byte kB1, Byte kB2, Byte kB3, Byte kB4, Byte kB5, Byte kB6, Byte kB7, Byte kB8, Byte kB9, Byte kBA, Byte kBB, Byte kBC, Byte kBD, Byte kBE, Byte kBF,
           Byte kC0, Byte kC1, Byte kC2, Byte kC3, Byte kC4, Byte kC5, Byte kC6, Byte kC7, Byte kC8, Byte kC9, Byte kCA, Byte kCB, Byte kCC, Byte kCD, Byte kCE, Byte kCF,
           Byte kD0, Byte kD1, Byte kD2, Byte kD3, Byte kD4, Byte kD5, Byte kD6, Byte kD7, Byte kD8, Byte kD9, Byte kDA, Byte kDB, Byte kDC, Byte kDD, Byte kDE, Byte kDF,
           Byte kE0, Byte kE1, Byte kE2, Byte kE3, Byte kE4, Byte kE5, Byte kE6, Byte kE7, Byte kE8, Byte kE9, Byte kEA, Byte kEB, Byte kEC, Byte kED, Byte kEE, Byte kEF,
           Byte kF0, Byte kF1, Byte kF2, Byte kF3, Byte kF4, Byte kF5, Byte kF6, Byte kF7, Byte kF8, Byte kF9, Byte kFA, Byte kFB, Byte kFC, Byte kFD, Byte kFE, Byte kFF);

private:
   Byte _enc[256], _dec[256];
};
/******************************************************************************/
struct AES // Advanced Encryption Standard
{
   Bool create(CPtr key, Int key_size); // create AES cipher, 'key'=pointer to symmetric key, 'key_size'=size of the key in bytes (must be 16, 24 or 32), returns false on fail

   void encrypt(Ptr dest, CPtr src)C; // encrypt 16-bytes from 'src' into 16-bytes in 'dest'
   void decrypt(Ptr dest, CPtr src)C; // decrypt 16-bytes from 'src' into 16-bytes in 'dest'

   AES();

private:
   ALIGN(16) U32 _encrypt_key[60] ALIGN_END(16);
   ALIGN(16) U32 _decrypt_key[60] ALIGN_END(16);
             Int _rounds;
}ALIGN_END(16); // this is needed for Apple
/******************************************************************************/
