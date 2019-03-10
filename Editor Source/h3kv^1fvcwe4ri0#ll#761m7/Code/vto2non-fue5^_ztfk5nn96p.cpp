/******************************************************************************/
const uint ProjectVersion     =64, // !! increase this by one if any of engine/editor asset formats have changed !!
           ClientServerVersion=59; // !! client/server version (client will fail if tries to connect to server compiled with different version), increase this by one if any of engine resource formats have changed or if the network protocol has changed or if editor classes formats have changed !!
const Str  ClientServerString ="Esenthel Editor";
/******************************************************************************/
const cchar8       *WorldVerSuffix     ="\\Data",
                   *ProjectsBuildPath  =  "_Build_\\", // !! this will     get deleted in 'CleanAll' !!
                   *ProjectsPublishPath="_Publish_\\", // !! this will     get deleted in 'CleanAll' !!
                   *ProjectsCodePath   =   "_Code_\\", // !! this will NOT get deleted in 'CleanAll' !!
                   *EsenthelProjectExt ="EsenthelProject", 
                   *CodeExt            =".cpp", // cpp was selected, because unlike other ideas: "code", "txt" only "cpp" gets correct coloring when opened in Visual Studio
                   *CodeSyncExt        =CodeExt;
const Str           NullName      ="<None>",
                    UnknownName   ="<Unknown>",
                    MultipleName  ="<Multiple Values>",
                    EsenthelStoreURL="http://www.esenthel.com/store.php";
const flt           StateFadeTime=0.2,
                    SkelSlotSize=0.2,
                    HeightmapTexScale=6,
                    WaypointRadius=0.5,
                    FlushWaypointsDelay=4,
                    FlushWaterDelay=4,
                    BuyFullVersionTime=10*60, // 10 min
                    AutoSaveTime=5*60, // 5 min
                    MaxGameViewRange    =2000,
                    MaxGameViewRangeDemo=400,
                    PropElmNameWidth=0.325, // value width of property having element name
                    SendAreasDelay=1.0,
                    VtxDupPosEps=0.0002, 
                    DefaultFOV=DegToRad(70),
                    PreviewFOV=DegToRad(50);
const bool          MiscOnTop=false,
                    CodeMenuOnTop=false,
                    ModeTabsAlwaysVisible=false,
                    RequireAllCodeMatchForSync=true,
                    TolerantSecondaryServer=false, // will ignore DeviceID when getting confirmation from secondary authentication server
                    SupportBC7           = true, // if support BC7 compression
                    WebBC7               =false, // if support BC7 compression for Web TODO: enable this once browsers start supporting BC7
                    AndroidETC2          =false, // if use ETC2 instead of ETC1 for Android TODO: enable this once OpenGL ES 3.0 is supported on majority of Android devices - https://developer.android.com/about/dashboards/index.html
                    ForceHQMtrlBase0     =false, // if always use high quality compression for Material Base0  Texture (RGBA/RGB Bump)
                    ForceHQMtrlBase1     = true, // if always use high quality compression for Material Base1  Texture (NxNySpecAlpha), set to true because normals need this (without this, they get very blocky due to low quality)
                    ForceHQMtrlDetail    = true, // if always use high quality compression for Material Detail Texture (NxNyColBump  ), set to true because normals need this (without this, they get very blocky due to low quality)
                     RemoveMtrlDetailBump= true, // if always delete Bump channel from Material Detail Texture (bump channel is not actually used in shaders, so removing it from the texture will give following benefits: ForceHQMtrlDetail ? will give better quality for Nrm and Col : will allow BC1 texture and 2x smaller size)
                    ImportRemovedElms=false, 
                    RenameAnimBonesOnSkelChange=true; // See also: FIND_ANIM_BY_NAME_ONLY in the Engine
const int           ForceInstaller=-2, // -2=disable and don't update, -1=disable, 0=auto, 1=enable (this is used only in Debug)
                    HeightBrushNoiseRes=256,
                    MtrlBrushSlots=14,
                    MaxDemoProjElms=64,
                    MaxVisibleRadius=(X64 ? 20 : 15),
                    MaxMaterialDownsize=3, // 0=full, 1=half, 2=quarter
                    ClientSendBufSize=3*1024*1024, // 3 MB
                    ServerSendBufSize=2*1024*1024, // 2 MB
                    InvalidCoordinate=SIGN_BIT,
                    RebuildBlockSize=6,
                    LinearAnimFpsLimit=12, // min number of FPS for an animation to be set as linear
                    EsenthelStoreMaxFileNum   =16, // max num of files for item
                    EsenthelStoreMaxFileSize  =255*1024*1024, // 255 MB max file size
                    EsenthelStoreFileChunked  =100*1024*1024, // 100 MB
                    EsenthelStoreFileChunkSize= 24*1024*1024, //  24 MB
                    EsenthelStoreMaxIconSize  =128*1024, // 128 KB
                    EsenthelStoreMaxImageSize =256*1024, // 256 KB
                    EsenthelStoreEngineLicense[]={1, 4, 5}, // Binary Monthly, Binary Yearly, Source Yearly
                    DefaultVorbisBitRate= 96*1000,
                    DefaultOpusBitRate  = 96*1000,
                    DefaultHQBitRate    =128*1000,
                    MeshSplitMaxSteps=3,    // max steps of splitting a mesh
                    MeshSplitMinVtxs=12000, // min number of vertexes in a mesh to split it
                    MeshSplitMinSize=4;     // min size of mesh box (in meters) to split it
const uint          MeshJoinAllTestVtxFlag=VTX_HLP|VTX_SIZE; // this is because of "Leaf" shader which works differently depending on existence of these components
const COMPRESS_TYPE ServerNetworkCompression     =COMPRESS_LZ4, ClientNetworkCompression     =COMPRESS_LZMA, EsenthelProjectCompression     =COMPRESS_LZMA;
const int           ServerNetworkCompressionLevel=9           , ClientNetworkCompressionLevel=5            , EsenthelProjectCompressionLevel=5;
const uint          NewElmTime=1; // use 1 instead of 0, so when downloading file, we have no 'param_value' and 'param_value_time' at 0, meaning we need to download the param
const cchar8       *      AppName="Esenthel Editor",
                   *ServerAppName="Esenthel Server",
                   *InstallerName="Esenthel Installer";
const cchar8       *ObjAccessNames[]=
{
   "Custom" , // 0
   "Terrain", // 1
   "Grass"  , // 2
   "Overlay", // 3
}; ASSERT(OBJ_ACCESS_CUSTOM==0 && OBJ_ACCESS_TERRAIN==1 && OBJ_ACCESS_GRASS==2 && OBJ_ACCESS_OVERLAY==3);
int ObjAccessNamesElms=Elms(ObjAccessNames);

const cchar8 *ElmNameMesh="mesh",
             *ElmNameSkel="skel",
             *ElmNamePhys="phys";

const uint EditMeshFlagAnd=~(VTX_DUP|EDGE_ALL|FACE_NRM|ADJ_ALL|VTX_TAN_BIN), // TanBin are not needed in Edit because they're always re-created if needed
           GameMeshFlagAnd=~(VTX_DUP|EDGE_ALL|FACE_NRM|ADJ_ALL);

const    ImagePtr    ImageNull;
const MaterialPtr MaterialNull;

bool          IsServer=false;
ReadWriteSync WorldAreaSync;
TimeStamp     CurTime; // it's set to the current time at the beginning of each frame, it is used to increase chances of 'AreaVer.old' at the moment of area send attempt
/******************************************************************************/
Str SDKPath() {return GetPath(App.exe());}
Str BinPath() {return SDKPath().tailSlash(true)+"Bin";}
/******************************************************************************/
enum SAVE_MODE
{
   SAVE_DEFAULT, // default save
   SAVE_AUTO   , //    auto-save
}
/******************************************************************************/
enum LOAD_RESULT
{
   LOAD_OK    , // success
   LOAD_EMPTY , // ok    , but project is empty
   LOAD_NEWER , // failed, project requires newer engine version
   LOAD_LOCKED, // failed, project is currently opened in another instance (or that instance closed without unlocking)
   LOAD_ERROR , // failed, error occured while reading the file
}
bool LoadOK(LOAD_RESULT result)
{
   return result==LOAD_OK || result==LOAD_EMPTY;
}
/******************************************************************************/
enum CIPHER_TYPE : byte
{
   CIPHER_NONE,
   CIPHER_1,
   CIPHER_2,
   CIPHER_3,
   CIPHER_NUM,
}
class ClassNameDesc
{
   cchar8 *clazz, *name;
   cchar  *desc;
}
ClassNameDesc CipherText[]=
{
   {null     , "No Encryption", u"Project data is not going to be encrypted (not recommended).\nAnyone will be able to access your project files.\nHowever this mode has the best performance x20"}, // CIPHER_NONE
   {"Cipher1", "Cipher 1"     , u"Project data is going to be encrypted using 'Cipher1' class (recommended).\nIt offers good security and good performance x2"}, // CIPHER_1
   {"Cipher2", "Cipher 2"     , u"Project data is going to be encrypted using 'Cipher2' class.\nIt offers great security and medium performance x1.5"}, // CIPHER_2
   {"Cipher3", "Cipher 3"     , u"Project data is going to be encrypted using 'Cipher3' class.\nIt offers best security and low performance x1"}, // CIPHER_3
}; ASSERT(CIPHER_NONE==0 && CIPHER_1==1 && CIPHER_2==2 && CIPHER_3==3 && CIPHER_NUM==4);
class ProjectCipher
{
   Cipher1  cipher1;
   Cipher2  cipher2;
   Cipher3  cipher3;
   Cipher  *cipher_ptr=null;
   
   Cipher*        operator()(        ) {return cipher_ptr;}
   ProjectCipher& clear(             ) {cipher_ptr=null; return T;}
   ProjectCipher& set  (Project &proj)
   {
      switch(proj.cipher)
      {
         default      : cipher_ptr=null; break; // CIPHER_NONE
         case CIPHER_1: cipher_ptr=&cipher1.setKey(proj.cipher_key, SIZE(proj.cipher_key)); break;
         case CIPHER_2: cipher_ptr=&cipher2.setKey(proj.cipher_key, SIZE(proj.cipher_key)); break;
         case CIPHER_3: cipher_ptr=&cipher3.setKey(proj.cipher_key, SIZE(proj.cipher_key)); break;
      }
      return T;
   }
}
/******************************************************************************/
enum USER_ACCESS : byte
{
   UA_NO_ACCESS,
   UA_READ_ONLY,
   UA_ARTIST   ,
   UA_NORMAL   ,
   UA_ADMIN    ,
   UA_NUM      ,
}
NameDesc UserAccessText[]=
{
   {u"No Access", u"User has no access at all (he's either awaiting activation or got blocked)"}, // UA_NO_ACCESS
   {u"Read Only", u"User can only read data from the server (he cannot upload anything)"}, // UA_READ_ONLY
   {u"Artist"   , u"User can read and upload data (with the exception of changing Codes and some Application properties)"}, // UA_ARTIST
   {u"Normal"   , u"User can read and upload data"}, // UA_NORMAL
   {u"Admin"    , u"User can read and upload data (extra permissions reserved for future use)"}, // UA_ADMIN
};
bool CanRead     (USER_ACCESS access) {return access>=UA_READ_ONLY;}
bool CanWrite    (USER_ACCESS access) {return access>=UA_ARTIST   ;}
bool CanWriteCode(USER_ACCESS access) {return access>=UA_NORMAL   ;}
/******************************************************************************/
class Version // value randomized each time the data is changed indicating that client/server sync is needed
{
   uint ver=0;

   bool operator==(C Version &v)C {return  ver==v.ver;}
   bool operator!=(C Version &v)C {return  ver!=v.ver;}
   bool operator! (            )C {return !ver;}
        operator bool(         )C {return  ver!=0;}

   void randomize() {uint old=ver; do ver=Random();while(!ver || ver==old);} // avoid zero and previous value
}
/******************************************************************************/
class TimeStamp
{
   static const long Start=63524217600, // 63524217600 is the number of seconds at 1st Jan 2013 (approximate time of the first application version)
                     Unix =62167219200; // 62167219200 is the number of seconds at 1st Jan 1970

   uint u=0;

   bool is()C {return u!=0;} // if was set

   uint text()C {return u;} // this method is used when saving to text, it can be replaced in the future to something like "Str text()C {return ..;}"

   TimeStamp& operator--(   ) {u--; return T;}
   TimeStamp& operator--(int) {u--; return T;}
   TimeStamp& operator++(   ) {u++; return T;}
   TimeStamp& operator++(int) {u++; return T;}

   TimeStamp& zero  () {u=0;                   return T;}
   TimeStamp& getUTC() {T=DateTime().getUTC(); return T;} // set to current time
   TimeStamp& now   () {uint prev=u; getUTC(); if(u<=prev)u=prev+1; return T;} // set to current time and make sure that it's newer than the previous time

   TimeStamp& fromUnix(long u) {T=u+(Unix-Start); return T;}

   bool old(C TimeStamp &now=TimeStamp().getUTC())C {return T<now;} // if this timestamp is older than 'now'

   bool operator==(C TimeStamp &t)C {return u==t.u;} // if equal
   bool operator!=(C TimeStamp &t)C {return u!=t.u;} // if not equal
   bool operator>=(C TimeStamp &t)C {return u>=t.u;} // if greater or equal
   bool operator<=(C TimeStamp &t)C {return u<=t.u;} // if smaller or equal
   bool operator> (C TimeStamp &t)C {return u> t.u;} // if greater
   bool operator< (C TimeStamp &t)C {return u< t.u;} // if smaller

   TimeStamp& operator+=(int i) {T=long(T.u)+long(i); return T;}
   TimeStamp& operator-=(int i) {T=long(T.u)-long(i); return T;}

   TimeStamp operator+(int i) {return long(T.u)+long(i);}
   TimeStamp operator-(int i) {return long(T.u)-long(i);}

   long operator-(C TimeStamp &t)C {return long(u)-long(t.u);}

   DateTime asDateTime()C {return DateTime().fromSeconds(u+Start);}

   TimeStamp(   int      i ) {T.u=Max(i, 0);}
   TimeStamp(  uint      u ) {T.u=u;}
   TimeStamp(  long      l ) {T.u=Mid(l, (long)0, (long)UINT_MAX);}
   TimeStamp(C DateTime &dt) {T=dt.seconds()-Start;}
   TimeStamp(C Str      &t ) {T.u=TextUInt(t);} // this method is used when loading from text, it can be replaced in the future

   static int Compare(C TimeStamp &a, C TimeStamp &b) {return .Compare(a.u, b.u);}
}
TimeStamp Min(C TimeStamp &a, C TimeStamp &b) {return a<=b ? a : b;}
TimeStamp Max(C TimeStamp &a, C TimeStamp &b) {return a>=b ? a : b;}
/******************************************************************************/
class Pose
{
   flt scale=1;
   Vec pos=0, rot=0;

   Matrix operator()()C {return MatrixD().setPosScale(pos, scale).rotateZ(rot.z).rotateXY(rot.x, rot.y);}

   Str asText()C {return S+"Scale:"+scale+", Pos:"+pos+", Rot:"+rot;}

   Pose& reset() {scale=1; pos.zero(); rot.zero(); return T;}

   Pose& operator+=(C Vec    &offset   ) {pos+=offset      ; return T;}
   Pose& operator*=(C Pose   &transform) {T=T()*transform(); return T;}
   Pose& operator*=(C Matrix &matrix   ) {T=T()*matrix     ; return T;}

   Pose(C Matrix &matrix)
   {
      MatrixD matrix_d=matrix;
      scale=matrix.x.length();
      rot  =matrix_d.angles();
      pos  =matrix_d.rotateY(-rot.y).rotateX(-rot.x).rotateZ(-rot.z).scale(1/scale).pos;
   }

   bool operator==(C Pose &t)C {return scale==t.scale && pos==t.pos && rot==t.rot;}
   bool operator!=(C Pose &t)C {return !(T==t);}
}
const Pose PoseIdentity;
/******************************************************************************/
/*class Matrix3_8 // Matrix3 stored using 8 bytes
{
   ushort orn    , // orientation                  (2 bytes total)
          size[3]; // size encoded to 16-bit float (6 bytes total)

   Vec  scale(               )C {return Vec(F16To32(size[0]), F16To32(size[1]), F16To32(size[2]));}
   void set  (Matrix3 &matrix)C {File f; f.readMem(&orn, SIZE(orn)).decOrient2(matrix); matrix.scaleL(scale());}
   void zero (               )  {Zero(T);}

   bool operator==(C Matrix3_8 &matrix)C {return orn==matrix.orn && size[0]==matrix.size[0] && size[1]==matrix.size[1] && size[2]==matrix.size[2];}
   bool operator!=(C Matrix3_8 &matrix)C {return orn!=matrix.orn || size[0]!=matrix.size[0] || size[1]!=matrix.size[1] || size[2]!=matrix.size[2];}

   Matrix3_8() {}
   Matrix3_8(C Matrix3 &matrix)
   {
      size[0]=F32To16(Mid(matrix.x.length(), (flt)-HALF_MAX, (flt)HALF_MAX));
      size[1]=F32To16(Mid(matrix.y.length(), (flt)-HALF_MAX, (flt)HALF_MAX));
      size[2]=F32To16(Mid(matrix.z.length(), (flt)-HALF_MAX, (flt)HALF_MAX));
      File f; f.writeMem().cmpOrient2(matrix).pos(0); f>>orn;
   }
}*/
class SmallMatrix3 // Matrix3 stored using 2xVec (2*3xFlt, 2*3*4xByte, 24 bytes)
{
   Vec angle,
       scale;

   bool operator==(C SmallMatrix3 &m)C {return angle==m.angle && scale==m.scale;}
   bool operator!=(C SmallMatrix3 &m)C {return angle!=m.angle || scale!=m.scale;}

   void set(Matrix3 &matrix)C {matrix.setRotateZ(angle.z).rotateXY(angle.x, angle.y).scaleL(scale);}

   SmallMatrix3() {}
   SmallMatrix3(C Matrix3 &matrix) {angle=matrix.angles(); scale=matrix.scale();}
}
class SmallMatrix : SmallMatrix3 // Matrix stored using 3xVec (3*3xFlt, 3*3*4xByte, 36 bytes)
{
   Vec pos;

   SmallMatrix3& orn()  {return T;}
 C SmallMatrix3& orn()C {return T;}

   bool operator==(C SmallMatrix &m)C {return orn()==m.orn() && pos==m.pos;}
   bool operator!=(C SmallMatrix &m)C {return orn()!=m.orn() || pos!=m.pos;}

   void set(Matrix &matrix)C {super.set(matrix); matrix.pos=pos;}

   Matrix operator()()C {Matrix matrix; set(matrix); return matrix;}

   SmallMatrix() {}
   SmallMatrix(C Matrix &matrix) : SmallMatrix3(matrix) {pos=matrix.pos;}
}
/******************************************************************************/
class NameDesc
{
   cchar *name, *desc;
}
/******************************************************************************/
class Chunk : Mems<byte>
{
   Str  name;
   uint ver=0;

   Chunk& create(C Str &name, uint ver, File &file) {T.name=name; T.ver=ver; file.get(setNum(file.left()).data(), file.left()); return T;}
   Chunk& create(ChunkReader &cr,       File &file) {return create(cr.name(), cr.ver(), file);}
}
class Chunks
{
   Memc<Chunk> chunks;

   static int Compare(C Chunk &a, C Chunk &b) {return ComparePath(a.name, b.name);}

   Chunk* findChunk(C Str &name) {REPA(chunks)if(EqualPath(chunks[i].name, name))return &chunks[i]; return null;}
   Chunks& delChunk(C Str &name) {REPA(chunks)if(EqualPath(chunks[i].name, name))chunks.remove(i, true); return T;}
   Chunks& setChunk(C Str &name, uint ver, File &file) {Chunk *chunk=findChunk(name); if(!chunk)chunk=&chunks.New(); chunk.create(name, ver, file); return T;}

   void del() {chunks.del();}

   // io
   bool load(File &f)
   {
      del();
      ChunkReader cr; if(cr.read(f))
      {
         for(; File *f=cr(); )chunks.New().create(cr, *f);
         return true;
      }
      return false;
   }
   bool save(File &f) // warning: this sorts 'chunks' and changes memory addresses for each element
   {
      ChunkWriter cw(f);
      chunks.sort(Compare);
      FREPA(chunks)
      {
         Chunk &chunk=chunks[i];
         if(File *f=cw.beginChunk(chunk.name, chunk.ver))f.put(chunk.data(), chunk.elms());
      }
      return f.ok();
   }
   bool load(C Str &name, ReadWriteSync &rws)
   {
      ReadLock rl(rws);
      File f; if(f.readTry(name))return load(f);
      del(); return false;
   }
   bool save(C Str &name, ReadWriteSync &rws) // warning: this sorts 'chunks' and changes memory addresses for each element
   {
      File f; if(save(f.writeMem())){f.pos(0); return SafeOverwriteChunk(f, name, rws);}
      return false;
   }
}
/******************************************************************************/
class MtrlImages
{
   bool  flip_normal_y=false;
   int   tex=0;
   Image color, alpha, bump, normal, specular, glow;
   
   bool create(C VecI2 &size)
   {
      flip_normal_y=false;
      tex=0;
      return color   .createSoftTry(size.x, size.y, 1, IMAGE_R8G8B8)
          && alpha   .createSoftTry(size.x, size.y, 1, IMAGE_I8)
          && bump    .createSoftTry(size.x, size.y, 1, IMAGE_I8)
          && normal  .createSoftTry(size.x, size.y, 1, IMAGE_R8G8B8)
          && specular.createSoftTry(size.x, size.y, 1, IMAGE_I8)
          && glow    .createSoftTry(size.x, size.y, 1, IMAGE_I8);
   }
   void clear()
   {
      flip_normal_y=false;
      tex=0;
      color   .clear();
      alpha   .clear();
      specular.clear();
      glow    .clear();

      REPD(y, bump.h())
      REPD(x, bump.w())bump.pixB(x, y)=128;

      Color nrm(128, 128, 255);
      REPD(y, normal.h())
      REPD(x, normal.w())normal.color(x, y, nrm);
   }
   void compact()
   {
      if(!(tex&BT_COLOR   ))color   .del();
      if(!(tex&BT_ALPHA   ))alpha   .del();
      if(!(tex&BT_BUMP    ))bump    .del();
      if(!(tex&BT_NORMAL  ))normal  .del();
      if(!(tex&BT_SPECULAR))specular.del();
      if(!(tex&BT_GLOW    ))glow    .del();
   }
   void Export(C Str &name, C Str &ext)C
   {
      color   .Export(name+"color."   +ext);
      alpha   .Export(name+"alpha."   +ext);
      bump    .Export(name+"bump."    +ext);
      normal  .Export(name+"normal."  +ext);
      specular.Export(name+"specular."+ext);
      glow    .Export(name+"glow."    +ext);
   }
   static void Crop(Image &image, C Rect &frac)
   {
      if(image.is())
      {
         RectI rect=Round(frac*Vec2(image.size()));
         Image temp; if(temp.createSoftTry(rect.w(), rect.h(), 1, image.compressed() ? IMAGE_R8G8B8A8 : image.type())) // crop manually because we need to use Mod
         {
            if(image.lockRead())
            {
               REPD(y, temp.h())
               REPD(x, temp.w())temp.color(x, y, image.color(Mod(x+rect.min.x, image.w()), Mod(y+rect.min.y, image.h())));
               image.unlock();
               Swap(temp, image);
            }
         }
      }
   }
   void crop(C Rect &frac)
   {
      Crop(color   , frac);
      Crop(alpha   , frac);
      Crop(bump    , frac);
      Crop(normal  , frac);
      Crop(specular, frac);
      Crop(glow    , frac);
   }
   void resize(C VecI2 &size)
   {
      if(size.x>=0 || size.y>=0)
      {
         if(color   .is())color   .resize(size.x, size.y);
         if(alpha   .is())alpha   .resize(size.x, size.y);
         if(bump    .is())bump    .resize(size.x, size.y);
         if(normal  .is())normal  .resize(size.x, size.y);
         if(specular.is())specular.resize(size.x, size.y);
         if(glow    .is())glow    .resize(size.x, size.y);
      }
   }
   void fromMaterial(C EditMaterial &material, C Project &proj, bool changed_flip_normal_y, C VecI2 &size=-1, bool process_alpha=false)
   {
      // !! here order of loading images is important, because we pass pointers to those images in subsequent loads !!
      bool col_ok=proj.loadImages(   color, material.   color_map, false),
         alpha_ok=proj.loadImages(   alpha, material.   alpha_map, false, WHITE, &color),
          spec_ok=proj.loadImages(specular, material.specular_map, false, WHITE, &color),
          bump_ok=proj.loadImages(    bump, material.    bump_map, false, GREY , &color, &specular),
           nrm_ok=proj.loadImages(  normal, material.  normal_map, false, Color(128, 128, 255), &color, &specular, &bump),
          glow_ok=proj.loadImages(    glow, material.    glow_map, false);

      if(! col_ok && !material. alpha_map.is())alpha_ok=false; // if color map failed to load, and there is no dedicated alpha  map, and since it's possible that alpha  was created from the color, which is not available, so alpha  needs to be marked as failed
      if(!bump_ok && !material.normal_map.is())  nrm_ok=false; // if bump  map failed to load, and there is no dedicated normal map, and since it's possible that normal was created from the bump , which is not available, so normal needs to be marked as failed

      ExtractBaseTextures(proj, material.base_0_tex, material.base_1_tex,
         col_ok ? null : &color, alpha_ok ? null : &alpha, bump_ok ? null : &bump, nrm_ok ? null : &normal, spec_ok ? null : &specular, glow_ok ? null : &glow, size);

      T.flip_normal_y=(nrm_ok ? material.flip_normal_y : changed_flip_normal_y); // if we failed to load the original image, and instead we're using extracted normal map, then we need to flip Y only if we're changing flipping at this moment

      bool keep_alpha=false;

      if(material.glow_map.is() && glow.is() && !material.alpha_map.is())alpha.del();else // if we have glow map specified and existing, and no specified alpha map, then delete alpha so glow can be used (this is necessary because alpha+glow maps are not compatible with each other)
         if(process_alpha)processAlpha();

      resize(size);
   }
   uint createBaseTextures(Image &base_0, Image &base_1)C
   {
      return CreateBaseTextures(base_0, base_1, color, alpha, bump, normal, specular, glow, true, flip_normal_y);
   }
   void baseTextureSizes(VecI2 *size0, VecI2 *size1)
   { // TODO: this could be optimized by calculating Max of image sizes, however there are some special cases (like glow isn't compatible with alpha, or normal made from bump, etc.)
      Image base[2]; createBaseTextures(base[0], base[1]);
      if(size0)*size0=base[0].size();
      if(size1)*size1=base[1].size();
   }
   void processAlpha()
   {
      if(!alpha.is() && ImageTI[color.type()].a) // if we have no alpha map but it's possible it's in color
         color.copyTry(alpha, -1, -1, -1, IMAGE_A8, IMAGE_SOFT, 1);

      if(alpha.is() && ImageTI[alpha.type()].channels>1 && ImageTI[alpha.type()].a) // if alpha has both RGB and Alpha channels, then check which one to use
         if(alpha.lockRead())
      {
         byte min_alpha=255, min_lum=255;
         REPD(y, alpha.h())
         REPD(x, alpha.w())
         {
            Color c=alpha.color(x, y);
            MIN(min_alpha, c.a    );
            MIN(min_lum  , c.lum());
         }
         alpha.unlock();
         if(min_alpha>=253 && min_lum>=253)alpha.del();else
         alpha.copyTry(alpha, -1, -1, -1, (min_alpha>=253 && min_lum<253) ? IMAGE_L8 : IMAGE_A8, IMAGE_SOFT, 1); // alpha channel is almost fully white -> use luminance as alpha
      }
   }
}
/******************************************************************************/
