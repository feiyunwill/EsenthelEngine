/******************************************************************************/
// FILE
/******************************************************************************/
bool SafeDel(C Str &name, ReadWriteSync &rws)
{
   if(name.is()){WriteLock wl(rws); return FDelFile(name);} return true;
}
bool SafeOverwrite(File &src, C Str &dest, ReadWriteSync &rws)
{
   return EE.SafeOverwrite(src, dest, null, null, S+'@'+TextHex(Random()), &rws);
}
bool SafeOverwriteChunk(File &src, C Str &dest, ReadWriteSync &rws)
{
   return (src.size()>4) ? SafeOverwrite(src, dest, rws) : SafeDel(dest, rws);
}
bool SafeCopy(C Str &src, C Str &dest)
{
   File f; return f.readStdTry(src) && EE.SafeOverwrite(f, dest, &NoTemp(FileInfoSystem(src).modify_time_utc), null, S+"@new"+Random());
}
/******************************************************************************/
void RemoveChunk(C Str &file, C Str &chunk, ReadWriteSync &rws)
{
   ReadLock rl(rws);
   File src; if(src.readTry(file))
   {
      File temp; ChunkWriter cw(temp.writeMem());
      for(ChunkReader cr(src); File *s=cr(); )
         if(!EqualPath(cr.name(), chunk))
            if(File *d=cw.beginChunk(cr.name(), cr.ver()))
               s.copy(*d);
      src.del();
      cw.endChunk();
      temp.pos(0); SafeOverwriteChunk(temp, file, rws);
   }
}
/******************************************************************************/
cchar8 *SizeSuffix[]={"", " KB", " MB", " GB", " TB"};
Str FileSize(long size, char dot=',')
{
   const int f=10;
   size*=f;
   int i=0; for(; i<Elms(SizeSuffix)-1 && size>=1000*f; i++, size>>=10); // check for "1000*f" instead of "1024*f", because we want to avoid displaying things like "1 001 MB"
   Str s=TextInt(size/f, -1, 3); if(size<100*f && i){s+=dot; s+=size%10;} s+=SizeSuffix[i];
   return s;
}
Str FileSizeKB(long size)
{
   const int f=10;
   size*=f;
   int i=1; size>>=10;
   Str s=TextInt(size/f, -1, 3); if(size<100*f && i){s+=','; s+=size%10;} s+=SizeSuffix[i];
   return s;
}
/******************************************************************************/
void SavedImage        (C Str &name) {if(ImagePtr       e=ImagePtr     ().find(name))if(!IsServer)e->load(name);} // on server the file may be compressed
void SavedImageAtlas   (C Str &name) {if(ImageAtlasPtr  e=ImageAtlasPtr().find(name))if(!IsServer)e->load(name);}
void SavedEditSkel     (C Str &name) {}
void SavedSkel         (C Str &name) {if(Skeleton      *e=Skeletons .find(name))if(!IsServer)e->load(name);}
void SavedAnim         (C Str &name) {if(Animation     *e=Animations.find(name))if(!IsServer)e->load(name);}
void SavedMesh         (C Str &name) {if(MeshPtr        e=MeshPtr ().find(name))if(!IsServer){CacheLock cl(Meshes); e->load(name);}}
void SavedEditMtrl     (C Str &name) {}
void SavedEditWaterMtrl(C Str &name) {}
void SavedEditPhysMtrl (C Str &name) {}
void SavedMtrl         (C Str &name) {if(MaterialPtr    e= MaterialPtr().find(name))if(!IsServer)e->load(name);}
void SavedWaterMtrl    (C Str &name) {if(WaterMtrlPtr   e=WaterMtrlPtr().find(name))if(!IsServer)e->load(name);}
void SavedPhysMtrl     (C Str &name) {if(PhysMtrl      *e=PhysMtrls     .find(name))if(!IsServer)e->load(name);}
void SavedEditPhys     (C Str &name) {}
void SavedPhys         (C Str &name) {if(PhysBodyPtr    e=PhysBodyPtr().find(name))if(!IsServer){CacheLock cl(PhysBodies); e->load(name);}}
void SavedEnum         (C Str &name) {if(Enum          *e=Enums        .find(name))if(!IsServer)e->load(name);}
void SavedEditEnum     (C Str &name) {}
void SavedEditObjPar   (C Str &name) {if(EditObjectPtr  e=EditObjectPtr().find(name))if(!IsServer)e->load(name);}
void SavedGameObjPar   (C Str &name) {if(ObjectPtr      e=    ObjectPtr().find(name))if(!IsServer){CacheLock cl(Objects); e->load(name);}}
void SavedGameWayp     (C Str &name) {if(Game.Waypoint *e=Game.Waypoints. find(name))if(!IsServer)e->load(name);}
void SavedFont         (C Str &name) {if(FontPtr        e=      FontPtr().find(name))if(!IsServer)e->load(name);}
void SavedTextStyle    (C Str &name) {if(TextStylePtr   e= TextStylePtr().find(name))if(!IsServer)e->load(name);}
void SavedPanelImage   (C Str &name) {if(PanelImagePtr  e=PanelImagePtr().find(name))if(!IsServer)e->load(name);}
void SavedPanel        (C Str &name) {if(PanelPtr       e=     PanelPtr().find(name))if(!IsServer)e->load(name);}
void SavedGuiSkin      (C Str &name) {if(GuiSkinPtr     e=   GuiSkinPtr().find(name))if(!IsServer)e->load(name);}
void SavedGui          (C Str &name) {}
void SavedEnv          (C Str &name) {if(EnvironmentPtr e=EnvironmentPtr().find(name))if(!IsServer)e->load(name);}

void Saved(C Image           &img , C Str &name) {if(ImagePtr e=ImagePtr().find(name))img.copy(*e);}
void Saved(C ImageAtlas      &img , C Str &name) {SavedImageAtlas(name);}
void Saved(C IconSettings    &icon, C Str &name) {}
void Saved(C EditSkeleton    &skel, C Str &name) {}
void Saved(C     Skeleton    &skel, C Str &name) {if(Skeleton  *e=Skeletons .find(name))*e=skel;}
void Saved(C Animation       &anim, C Str &name) {if(Animation *e=Animations.find(name))*e=anim;}
void Saved(C Mesh            &mesh, C Str &name) {if(MeshPtr    e=MeshPtr ().find(name)){CacheLock cl(Meshes); e->create(mesh).setShader();}}
void Saved(C EditMaterial    &mtrl, C Str &name) {}
void Saved(C EditWaterMtrl   &mtrl, C Str &name) {}
void Saved(C EditPhysMtrl    &mtrl, C Str &name) {}
void Saved(C Material        &mtrl, C Str &name) {if( MaterialPtr e= MaterialPtr().find(name))*e=mtrl;}
void Saved(C WaterMtrl       &mtrl, C Str &name) {if(WaterMtrlPtr e=WaterMtrlPtr().find(name))*e=mtrl;}
void Saved(C PhysMtrl        &mtrl, C Str &name) {SavedPhysMtrl(name);}
void Saved(C PhysBody        &phys, C Str &name) {if(PhysBodyPtr  e=PhysBodyPtr().find(name)){CacheLock cl(PhysBodies); *e=phys;}}
void Saved(C Enum            &enm , C Str &name) {if(Enum        *e=Enums        .find(name))*e=enm;}
void Saved(C EditEnums       &enms, C Str &name) {}
void Saved(C EditObject      &obj , C Str &name) {if(EditObjectPtr e=EditObjectPtr().find(name))*e=obj;}
void Saved(C     Object      &obj , C Str &name) {if(    ObjectPtr e=    ObjectPtr().find(name)){CacheLock cl(Objects); *e=obj;}}
void Saved(C EditWaypoint    &wp  , C Str &name) {}
void Saved(C Game.Waypoint   &wp  , C Str &name) {if(Game.Waypoint *e=Game.Waypoints.find(name))*e=wp;}
void Saved(C EditFont        &font, C Str &name) {}
void Saved(C Font            &font, C Str &name) {SavedFont(name);}
void Saved(C EditTextStyle   &ts  , C Str &name) {}
void Saved(C EditPanelImage  &pi  , C Str &name) {}
void Saved(C PanelImage      &pi  , C Str &name) {SavedPanelImage(name);}
void Saved(C TextStyle       &ts  , C Str &name) {if(TextStylePtr e=TextStylePtr().find(name))*e=ts;}
void Saved(C EditPanel       &panl, C Str &name) {}
void Saved(C Panel           &panl, C Str &name) {if(PanelPtr     e=PanelPtr().find(name))*e=panl;}
void Saved(C EditGuiSkin     &skin, C Str &name) {}
void Saved(C     GuiSkin     &skin, C Str &name) {if(GuiSkinPtr   e=GuiSkinPtr().find(name))*e=skin;}
void Saved(C GuiObjs         &gui , C Str &name) {}
void Saved(C Lake            &lake, C Str &name) {}
void Saved(C River           &rivr, C Str &name) {}
void Saved(C EditEnv         &env , C Str &name) {}
void Saved(C Environment     &env , C Str &name) {if(EnvironmentPtr e=EnvironmentPtr().find(name))*e=env;}
void Saved(C Game.WorldSettings &s, C Str &name) {}

bool Save (C Image           &img , C Str &name                        ) {File f; img .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name)){Saved(img , name); return true;} return false;}
void Save (C ImageAtlas      &img , C Str &name                        ) {File f; img .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(img , name);}
void Save (C IconSettings    &icon, C Str &name                        ) {File f; icon.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(icon, name);}
void Save (C EditSkeleton    &skel, C Str &name                        ) {File f; skel.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(skel, name);}
void Save (C     Skeleton    &skel, C Str &name                        ) {File f; skel.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(skel, name);}
void Save (C Animation       &anim, C Str &name                        ) {File f; anim.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(anim, name);}
void Save (C Mesh            &mesh, C Str &name, C Str &resource_path=S) {File f; mesh.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); CacheLock cl(Meshes); if(SafeOverwrite(f, name))Saved(mesh, name);}
void Save (C EditMaterial    &mtrl, C Str &name                        ) {File f; mtrl.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C EditWaterMtrl   &mtrl, C Str &name                        ) {File f; mtrl.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C EditPhysMtrl    &mtrl, C Str &name                        ) {File f; mtrl.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C Material        &mtrl, C Str &name, C Str &resource_path=S) {File f; mtrl.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C WaterMtrl       &mtrl, C Str &name, C Str &resource_path=S) {File f; mtrl.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C PhysMtrl        &mtrl, C Str &name                        ) {File f; mtrl.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(mtrl, name);}
void Save (C PhysBody        &phys, C Str &name, C Str &resource_path=S) {File f; phys.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); CacheLock cl(PhysBodies); if(SafeOverwrite(f, name))Saved(phys, name);}
void Save (C Enum            &enm , C Str &name                        ) {File f; enm .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(enm , name);}
void Save (C EditEnums       &enms, C Str &name                        ) {File f; enms.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(enms, name);}
void Save (C EditObject      &obj , C Str &name, C Str &resource_path=S) {File f; obj .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(obj , name);}
void Save (C     Object      &obj , C Str &name, C Str &resource_path=S) {File f; obj .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); CacheLock cl(Objects); if(SafeOverwrite(f, name))Saved(obj, name);}
void Save (C EditWaypoint    &wp  , C Str &name                        ) {File f; wp  .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(wp  , name);}
void Save (C Game.Waypoint   &wp  , C Str &name                        ) {File f; wp  .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(wp  , name);}
void Save (C EditFont        &font, C Str &name                        ) {File f; font.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(font, name);}
void Save (C Font            &font, C Str &name                        ) {File f; font.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(font, name);}
void Save (C EditTextStyle   &ts  , C Str &name                        ) {File f; ts  .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(ts  , name);}
void Save (C TextStyle       &ts  , C Str &name, C Str &resource_path=S) {File f; ts  .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(ts  , name);}
void Save (C EditPanelImage  &pi  , C Str &name                        ) {File f; pi  .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(pi  , name);}
void Save (C PanelImage      &pi  , C Str &name                        ) {File f; pi  .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(pi  , name);}
void Save (C EditPanel       &panl, C Str &name                        ) {File f; panl.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(panl, name);}
void Save (C Panel           &panl, C Str &name, C Str &resource_path=S) {File f; panl.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(panl, name);}
void Save (C EditGuiSkin     &skin, C Str &name                        ) {File f; skin.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(skin, name);}
void Save (C     GuiSkin     &skin, C Str &name, C Str &resource_path=S) {File f; skin.save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(skin, name);}
void Save (C GuiObjs         &gui , C Str &name, C Str &resource_path=S) {File f; gui .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(gui , name);}
void Save (C Lake            &lake, C Str &name                        ) {File f; lake.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name, WorldAreaSync))Saved(lake, name);}
void Save (C River           &rivr, C Str &name                        ) {File f; rivr.save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name, WorldAreaSync))Saved(rivr, name);}
void Save (C EditEnv         &env , C Str &name                        ) {File f; env .save(f.writeMem()                                                    ); f.pos(0); if(SafeOverwrite(f, name))Saved(env , name);}
void Save (C Environment     &env , C Str &name, C Str &resource_path=S) {File f; env .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(env , name);}
void Save (C Game.WorldSettings &s, C Str &name, C Str &resource_path=S) {File f; s   .save(f.writeMem(), resource_path.is() ? resource_path : GetPath(name)); f.pos(0); if(SafeOverwrite(f, name))Saved(s   , name);}

bool Load (Mesh       &mesh, C Str &name, C Str &resource_path=S) {File f; if(f.readTry(name))return mesh.load(f, resource_path.is() ? resource_path : GetPath(name)); mesh.del  (); return false;}
bool Load (Material   &mtrl, C Str &name, C Str &resource_path=S) {File f; if(f.readTry(name))return mtrl.load(f, resource_path.is() ? resource_path : GetPath(name)); mtrl.reset(); return false;}
bool Load (WaterMtrl  &mtrl, C Str &name, C Str &resource_path=S) {File f; if(f.readTry(name))return mtrl.load(f, resource_path.is() ? resource_path : GetPath(name)); mtrl.reset(); return false;}
bool Load (EditObject &obj , C Str &name, C Str &resource_path=S) {File f; if(f.readTry(name))return obj .load(f, resource_path.is() ? resource_path : GetPath(name)); obj .del  (); return false;}
// other assets either don't use sub-assets, or are stored in game path and don't require "edit->game" path change

bool SaveCode(C Str &code, C Str &name)
{
 //FileText f; f.writeMem(HasUnicode(code) ? UTF_16 : ANSI); // avoid UTF_8 because it's slower to write/read, and as there can be lot of codes, we don't want to sacrifice performance when opening big projects
   FileText f; f.writeMem(HasUnicode(code) ? UTF_8 : ANSI); // FIXME restore above UTF_16 once github supports it, because now it can corrupt files
   f.fix_new_line=false; // don't write '\r' to reduce size and improve performance for saving/loading
   f.putText(code);
   return EE.SafeOverwrite(f, name);
}
Edit.ERROR_TYPE LoadCode(Str &code, C Str &name)
{
   FileText f; if(f.read(name)){f.getAll(code); return f.ok() ? Edit.EE_ERR_NONE : Edit.EE_ERR_FILE_READ_ERROR;}
   code.clear(); return Edit.EE_ERR_FILE_NOT_FOUND;
}

void SavedBase(ELM_TYPE type, C Str &path) // called when saved the base version
{
   if(ElmEdit(type))SavedEdit(type, path);
   else             SavedGame(type, path);
}
void SavedCode(C Str &path) // called when saved code
{
   
}
void SavedEdit(ELM_TYPE type, C Str &path) // called when saved the edit version
{
   switch(type)
   {
      case ELM_SKEL      : SavedEditSkel     (path); break;
      case ELM_PHYS      : SavedEditPhys     (path); break;
      case ELM_ENUM      : SavedEditEnum     (path); break;
      case ELM_OBJ_CLASS : SavedEditObjPar   (path); break;
      case ELM_OBJ       : SavedEditObjPar   (path); break;
      case ELM_MESH      : SavedMesh         (path); break;
      case ELM_MTRL      : SavedEditMtrl     (path); break;
      case ELM_WATER_MTRL: SavedEditWaterMtrl(path); break;
      case ELM_PHYS_MTRL : SavedEditPhysMtrl (path); break;
   }
}
void SavedGame(ELM_TYPE type, C Str &path) // called when saved the game version
{
   switch(type)
   {
      case ELM_ENUM       : SavedEnum      (path); break;
      case ELM_OBJ_CLASS  : SavedGameObjPar(path); break;
      case ELM_OBJ        : SavedGameObjPar(path); break;
      case ELM_MESH       : SavedMesh      (path); break;
      case ELM_MTRL       : SavedMtrl      (path); break;
      case ELM_WATER_MTRL : SavedWaterMtrl (path); break;
      case ELM_PHYS_MTRL  : SavedPhysMtrl  (path); break;
      case ELM_SKEL       : SavedSkel      (path); break;
      case ELM_PHYS       : SavedPhys      (path); break;
      case ELM_ANIM       : SavedAnim      (path); break;
      case ELM_GUI_SKIN   : SavedGuiSkin   (path); break;
      case ELM_GUI        : SavedGui       (path); break;
      case ELM_FONT       : SavedFont      (path); break;
      case ELM_TEXT_STYLE : SavedTextStyle (path); break;
      case ELM_PANEL_IMAGE: SavedPanelImage(path); break;
      case ELM_PANEL      : SavedPanel     (path); break;
      case ELM_ENV        : SavedEnv       (path); break;
      case ELM_IMAGE      : SavedImage     (path); break;
      case ELM_IMAGE_ATLAS: SavedImageAtlas(path); break;
      case ELM_ICON       : SavedImage     (path); break;
   }
}
/******************************************************************************/
class FileSizeGetter
{
   class Elm
   {
      UID  id;
      uint file_size;
   }
   Memc<Elm> elms;

   // get
   bool created()C {           return path.is();}
   bool busy   ()  {cleanup(); return thread.active();}
   bool get    ()
   {
      cleanup();
      if(elms_thread.elms())
      {
         SyncLocker locker(lock);
         if(!elms.elms())Swap(elms_thread, elms);else
         {
            FREPA(elms_thread)elms.add(elms_thread[i]);
            elms_thread.clear();
         }
         return true;
      }
      return false;
   }

   // manage
   void clear() {elms.clear();}
   void stop () {thread.stop();}
   void del  ()
   {
      thread.del(); // del the thread first
      elms_thread.clear();
      elms       .clear();
      path       .clear();
   }
   void get(C Str &path)
   {
      del();
      if(path.is())
      {
         T.path=path;
         thread.create(Func, this);
      }
   }
  ~FileSizeGetter() {del();}

private:
   Str       path;
   Memc<Elm> elms_thread;
   SyncLock  lock;
   Thread    thread;

   void cleanup()
   {
      if(!thread.active())thread.del(); // delete to free resources
   }

   static bool Func(Thread &thread) {return ((FileSizeGetter*)thread.user).func();}
          bool func()
   {
      for(FileFind ff(path); !thread.wantStop() && ff(); )
      {
         if(ff.type==FSTD_FILE)
         {
            UID id; if(DecodeFileName(ff.name, id))
            {
               SyncLocker locker(lock);
               Elm &elm=elms_thread.New();
               elm.id=id;
               elm.file_size=ff.size;
            }
         }
      }
      return false;
   }
}
/******************************************************************************/
// SYNC / UNDO
/******************************************************************************/
void MAX1(TimeStamp &time, C TimeStamp &src_time) {if(src_time>time)time=src_time; time++;} // set as max from both and increase by one, "time=Max(time, src_time)+1"

bool Sync(TimeStamp &time, C TimeStamp &src_time) {if(src_time> time){time=src_time;        return true;} return false;}
bool Undo(TimeStamp &time, C TimeStamp &src_time) {if(src_time!=time){MAX1(time, src_time); return true;} return false;}

<TYPE> bool Sync(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(Sync(time, src_time)){data=src_data; return true;} return false;
}
<TYPE> bool UndoByTime(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(Undo(time, src_time)){data=src_data; return true;} return false;
}

<TYPE> bool SyncByValue(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(data!=src_data){data=src_data; time=src_time; return true;} return false;
}
<TYPE> bool SyncByValueEqual(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(!Equal(data, src_data)){data=src_data; time=src_time; return true;} return false;
}

<TYPE> bool UndoByValue(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(data!=src_data){data=src_data; MAX1(time, src_time); return true;} return false;
}
/*<TYPE> bool UndoByValueEqual(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data)
{
   if(!Equal(data, src_data)){data=src_data; MAX1(time, src_time); return true;} return false;
}*/

<TYPE> bool Undo(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data) // ByTimeAndValue, make this the default function because there can be a lot of changes in the same second on the local computer
{
   return UndoByTime (time, src_time, data, src_data) // first check by time because it's faster
       || UndoByValue(time, src_time, data, src_data);
}
/*<TYPE> bool UndoEqual(TimeStamp &time, C TimeStamp &src_time, TYPE &data, C TYPE &src_data) // ByTimeAndValue
{
   return UndoByTime      (time, src_time, data, src_data) // first check by time because it's faster
       || UndoByValueEqual(time, src_time, data, src_data);
}*/

void SetUndo(C Edit._Undo &undos, Button &undo, Button &redo)
{
   undo.enabled(undos.undosAvailable());
   redo.enabled(undos.redosAvailable());
}
/******************************************************************************/
// IMAGE
/******************************************************************************/
DIR_ENUM GetCubeDir(int face)
{
   switch(face)
   {
      default: return DIR_LEFT;
      case  1: return DIR_FORWARD;
      case  2: return DIR_RIGHT;
      case  3: return DIR_BACK;
      case  4: return DIR_DOWN;
      case  5: return DIR_UP;
   }
}
Str GetCubeFile(C Str &files, int face)
{
   Mems<Edit.FileParams> faces=Edit.FileParams.Decode(files);
   return (faces.elms()==1) ? files : InRange(face, faces) ? faces[face].encode() : S;
}
Str SetCubeFile(Str files, int face, C Str &file) // put 'file' into specified 'face' and return all files
{
   if(InRange(face, 6))
   {
      Mems<Edit.FileParams> faces=Edit.FileParams.Decode(files);
      if(faces.elms()==1){faces.setNum(6); REPAO(faces)=files;} // set all from original
      if(faces.elms()!=6)faces.clear(); // if invalid number then clear
      faces.setNum(6); // set 6 faces
      faces[face]=file; // set i-th face to target file
      files=Edit.FileParams.Encode(faces); // get all faces
   }
   return files;
}
/******************************************************************************/
bool HasAlpha(C Image &image) // if image has alpha channel
{
   if(!ImageTI[image.type()].a)return false;
   Vec4 min, max; if(image.stats(&min, &max, null))return !(Equal(min.w, 1, 2.5/255) && Equal(max.w, 1, 2.5/255));
   return true;
}
bool HasColor(C Image &image) // if image is not monochromatic
{
   if(!ImageTI[image.type()].r && !ImageTI[image.type()].g && !ImageTI[image.type()].b)return false;
   if(image.type()==IMAGE_L8 || image.type()==IMAGE_L8A8)return false;
   Image temp; C Image *src=&image; if(ImageTI[src.hwType()].compressed)if(src.copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&temp;else return true;
   FREPD(face, (src.mode()==IMAGE_CUBE) ? 6 : 1)if(src.lockRead(0, DIR_ENUM(face)))
   {
      REPD(z, src.ld())
      REPD(y, src.lh())
      REPD(x, src.lw())
      {
         Color c=src.color3D(x, y, z); if(Abs(c.r-c.g)>1 || Abs(c.r-c.b)>1){src.unlock(); return true;}
      }
      src.unlock();
   }
   return false;
}
bool NeedFixAlpha(Image &image, IMAGE_TYPE type, bool always=false)
{
   return ((image.type()!=IMAGE_BC1 && type==IMAGE_BC1) || always) // if we're converting from non-BC1 to BC1, then since BC1 will make black pixels for alpha<128, we need to force full alpha (ETC2_A1 does that too, however if we didn't want it, we can just use ETC2)
       && ImageTI[image.type()].a;                                 // no point in chaning alpha if the source doesn't have it
}
bool FixAlpha(Image &image, IMAGE_TYPE type, bool always=false) // returns if any change was made
{
   if(NeedFixAlpha(image, type, always))
   {
      if(ImageTI[image.hwType()].compressed)return image.copyTry(image, -1, -1, -1, IMAGE_R8G8B8, IMAGE_SOFT, 1);
      if(image.lock())
      {
         REPD(y, image.h())
         REPD(x, image.w()){Color c=image.color(x, y); c.a=255; image.color(x, y, c);}
         image.unlock();
         return true;
      }
   }
   return false;
}

enum
{
   FORCE_HQ    =1<<0, // high quality
   IGNORE_ALPHA=1<<1,
}
void ImageProps(C Image &image, UID *md5, IMAGE_TYPE *compress_type=null, uint flags=0) // calculate image MD5 (when in IMAGE_R8G8B8A8 type) and get best type for image compression
{
   if((flags&FORCE_HQ    ) && compress_type){*compress_type=IMAGE_BC7; compress_type=null;} // when forcing  HQ    set BC7 and set null so it no longer needs to be calculated, check this before IGNORE_ALPHA
   if((flags&IGNORE_ALPHA) && compress_type){*compress_type=IMAGE_BC1; compress_type=null;} // when ignoring alpha set BC1 and set null so it no longer needs to be calculated
   if(md5 || compress_type)
   {
      // set initial values
      if(md5          )md5.zero();
      if(compress_type)*compress_type=(SupportBC7 ? IMAGE_BC7 : IMAGE_BC3);

      // calculate
      if(!image.is())return;
      MD5   m;
      bool  bc1=true, bc2=true, // BC1 uses 1 bit alpha (0 or 255), BC2 uses 4 bit alpha
            force_alpha=(md5 && (flags&IGNORE_ALPHA) && ImageTI[image.type()].a); // if we want hash and we want to ignore alpha, and source had alpha, then we need to adjust as if it has full alpha, this is done because: ignoring alpha may save the image in format that doesn't support the alpha channel, however if the same image is later used for something else, and now wants to use that alpha channel, then it needs to be created as a different texture (with different hash)
      FREPD(face, (image.mode()==IMAGE_CUBE) ? 6 : 1)
      {
         Image temp; C Image *src=&image;
         if((md5 && src.hwType()!=IMAGE_R8G8B8A8) // calculating hash requires RGBA format
         || (compress_type && ImageTI[src.hwType()].compressed) // checking compress_type requires color reads so copy to RGBA soft to make them faster
         || force_alpha) // forcing alpha requires modifying the alpha channel, so copy to 'temp' which we can modify
            if(src->extractMipMap(temp, IMAGE_R8G8B8A8, IMAGE_SOFT, 0, DIR_ENUM(face)))src=&temp;else return;
         if(src.lockRead(0, DIR_ENUM(face)))
         {
            if(force_alpha  ) REPD(z, temp.d())
                              REPD(y, temp.h())
                              REPD(x, temp.w())temp.pixC(x, y, z).a=255; // set before calculating hash
            if(md5          )FREPD(z,  src.d())
                             FREPD(y,  src.h())m.update(src.data() + y*src.pitch() + z*src.pitch2(), src.w()*4);
            if(compress_type) REPD(z,  src.d())
                              REPD(y,  src.h())
                              REPD(x,  src.w())
            {
               Color c=src.color3D(x, y, z);
               byte  bc2_a=((c.a*15+128)/255)*255/15;
               if(c.a> 1 && c.a<254            // BC1 supports only 0 and 255 alpha
               || c.a==0 && c.lum())bc1=false; // BC1 supports only black color at 0 alpha
               if(Abs(c.a-bc2_a)>1 )bc2=false;
            }
            src.unlock();
         }
      }

      if(md5          )*md5          =m();
      if(compress_type)*compress_type=(bc1 ? IMAGE_BC1 : SupportBC7 ? IMAGE_BC7 : bc2 ? IMAGE_BC2 : IMAGE_BC3); // prefer BC1 because it's 4-bit per pixel
   }
}
/******************************************************************************/
void LoadTexture(C Project &proj, C UID &tex_id, Image &image, C VecI2 &size=-1)
{
   ImagePtr src=proj.texPath(tex_id);
   if(src)src->copyTry(image, size.x, size.y, 1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1);else image.del(); // always copy, because: src texture will always be compressed, also soft doesn't require locking
}
void ExtractBaseTextures(C Project &proj, C UID &base_0, C UID &base_1, Image *col, Image *alpha, Image *bump, Image *normal, Image *specular, Image *glow, C VecI2 &size=-1)
{
   uint tex=0;
   if(base_0.valid() && base_1.valid()) // both textures specified
   {
      if(col || bump)
      {
         Image b0; LoadTexture(proj, base_0, b0, size);
         if(col )col .createSoft(b0.w(), b0.h(), 1, IMAGE_R8G8B8);
         if(bump)bump.createSoft(b0.w(), b0.h(), 1, IMAGE_L8);
         REPD(y, b0.h())
         REPD(x, b0.w())
         {
            Color c=b0.color(x, y);
            if(col ){col .color(x, y, c  ); if(c.r<254 || c.g<254 || c.b<254)tex|=BT_COLOR;}
            if(bump){bump.pixel(x, y, c.a); if(Abs(c.a-128)>1     && c.a<254)tex|=BT_BUMP ;} // BUMP_DEFAULT can be either 128 or 255
         }
      }
      if(alpha || normal || specular || glow)
      {
         Image b1; LoadTexture(proj, base_1, b1, size);
         if(alpha   )alpha   .createSoft(b1.w(), b1.h(), 1, IMAGE_L8);
         if(normal  )normal  .createSoft(b1.w(), b1.h(), 1, IMAGE_R8G8B8);
         if(specular)specular.createSoft(b1.w(), b1.h(), 1, IMAGE_L8);
         if(glow    )glow    .createSoft(b1.w(), b1.h(), 1, IMAGE_L8);
         REPD(y, b1.h())
         REPD(x, b1.w())
         {
            Color c=b1.color(x, y); // #MaterialTextureChannelOrder
          /*if(old)
            {
               if(alpha   ){alpha   .pixel(x, y, c.b); if(c.b<254)tex|=BT_ALPHA   ;}
               if(glow    ){glow    .pixel(x, y, c.b); if(c.b<254)tex|=BT_GLOW    ;}
               if(specular){specular.pixel(x, y, c.r); if(c.r<254)tex|=BT_SPECULAR;}
               if(normal  )
               {
                  Vec n; n.xy.set((c.a-128)/127.0, (c.g-128)/127.0); n.z=Sqrt(1 - n.x*n.x - n.y*n.y);
                  normal.color(x, y, Color(c.a, c.g, Mid(Round(n.z*127+128), 0, 255))); if(Abs(c.a-128)>1 || Abs(c.g-128)>1)tex|=BT_NORMAL;
               }
            }else*/
            {
               if(alpha   ){alpha   .pixel(x, y, c.a); if(c.a<254)tex|=BT_ALPHA   ;}
               if(glow    ){glow    .pixel(x, y, c.a); if(c.a<254)tex|=BT_GLOW    ;}
               if(specular){specular.pixel(x, y, c.b); if(c.b<254)tex|=BT_SPECULAR;}
               if(normal  )
               {
                  Vec n; n.xy.set((c.r-128)/127.0, (c.g-128)/127.0); n.z=Sqrt(1 - n.x*n.x - n.y*n.y);
                  normal.color(x, y, Color(c.r, c.g, Mid(Round(n.z*127+128), 0, 255))); if(Abs(c.r-128)>1 || Abs(c.g-128)>1)tex|=BT_NORMAL;
               }
            }
         }
      }
   }else
   if(base_0.valid()) // only one texture specified
   {
      if(col || alpha)
      {
         Image b0; LoadTexture(proj, base_0, b0, size);
         if(col  )col  .createSoft(b0.w(), b0.h(), 1, IMAGE_R8G8B8);
         if(alpha)alpha.createSoft(b0.w(), b0.h(), 1, IMAGE_L8);
         REPD(y, b0.h())
         REPD(x, b0.w())
         {
            Color c=b0.color(x, y);
            if(col  ){col  .color(x, y, c  ); if(c.r<254 || c.g<254 || c.b<254)tex|=BT_COLOR;}
            if(alpha){alpha.pixel(x, y, c.a); if(c.a<254                      )tex|=BT_ALPHA;}
         }
      }
   }
   if(col      && !(tex&BT_COLOR   ))col     .del();
   if(alpha    && !(tex&BT_ALPHA   ))alpha   .del();
   if(bump     && !(tex&BT_BUMP    ))bump    .del();
   if(normal   && !(tex&BT_NORMAL  ))normal  .del();
   if(specular && !(tex&BT_SPECULAR))specular.del();
   if(glow     && !(tex&BT_GLOW    ))glow    .del();
}
void ExtractDetailTexture(C Project &proj, C UID &detail_tex, Image *col, Image *bump, Image *normal)
{
   uint tex=0;
   if(detail_tex.valid())
      if(col || bump || normal)
   {
      Image det; LoadTexture(proj, detail_tex, det);
      if(col   )col   .createSoft(det.w(), det.h(), 1, IMAGE_L8);
      if(bump  )bump  .createSoft(det.w(), det.h(), 1, IMAGE_L8);
      if(normal)normal.createSoft(det.w(), det.h(), 1, IMAGE_R8G8B8);
      REPD(y, det.h())
      REPD(x, det.w())
      {
         Color c=det.color(x, y); // #MaterialTextureChannelOrder
       /*if(old)
         {
            if(col   ){col .pixel(x, y, c.r); if(c.r<254)tex|=BT_COLOR;}
            if(bump  ){bump.pixel(x, y, c.b); if(c.b<254)tex|=BT_BUMP ;}
            if(normal)
            {
               Vec n; n.xy.set((c.a-128)/127.0, (c.g-128)/127.0); n.z=Sqrt(1 - n.x*n.x - n.y*n.y);
               normal.color(x, y, Color(c.a, c.g, Mid(Round(n.z*127+128), 0, 255))); if(Abs(c.a-128)>1 || Abs(c.g-128)>1)tex|=BT_NORMAL;
            }
         }else*/
         {
            if(col   ){col .pixel(x, y, c.b); if(c.b<254)tex|=BT_COLOR;}
            if(bump  ){bump.pixel(x, y, c.a); if(c.a<254)tex|=BT_BUMP ;}
            if(normal)
            {
               Vec n; n.xy.set((c.r-128)/127.0, (c.g-128)/127.0); n.z=Sqrt(1 - n.x*n.x - n.y*n.y);
               normal.color(x, y, Color(c.r, c.g, Mid(Round(n.z*127+128), 0, 255))); if(Abs(c.r-128)>1 || Abs(c.g-128)>1)tex|=BT_NORMAL;
            }
         }
      }
   }
   if(col    && !(tex&BT_COLOR ))col   .del();
   if(bump   && !(tex&BT_BUMP  ))bump  .del();
   if(normal && !(tex&BT_NORMAL))normal.del();
}
UID MergedBaseTexturesID(C UID &base_0, C UID &base_1) // this function generates ID of a merged texture created from two base textures, formula for this function can be freely modified as in worst case merged textures will just get regenerated
{
   MD5 id;
   id.update(&base_0, SIZE(base_0));
   id.update(&base_1, SIZE(base_1));
   return id();
}
/******************************************************************************/
VecI ImageSize(C VecI &src, C VecI2 &custom, bool pow2)
{
   VecI size=src;
   if( custom.x>0)size.x=custom.x;
   if( custom.y>0)size.y=custom.y;
   if(!custom.x && custom.y>0 && src.y)size.x=Max(1, (src.x*custom.y+src.y/2)/src.y); // keep aspect ratio
   if(!custom.y && custom.x>0 && src.x)size.y=Max(1, (src.y*custom.x+src.x/2)/src.x); // keep aspect ratio
   if(pow2)size.set(NearestPow2(size.x), NearestPow2(size.y), NearestPow2(size.z));
   return size;
}
bool EditToGameImage(Image &edit, Image &game, bool pow2, bool alpha_lum, ElmImage.TYPE type, int mode, int mip_maps, bool has_color, bool has_alpha, bool ignore_alpha, C VecI2 &custom_size=0, C int *force_type=null)
{
   VecI size=ImageSize(edit.size3(), custom_size, pow2);

   Image temp, *src=&edit;

   if(force_type)
   {
      if(*force_type==IMAGE_NONE || *force_type<0)force_type=null;
   }
   if(alpha_lum)
   {
      if(&edit!=&game){src.copyTry(temp); src=&temp;}
      src.alphaFromBrightness().divRgbByAlpha();
   }
   if(ignore_alpha && ImageTI[src.type()].a) // if want to ignore alpha then set it to full as some compressed texture formats will benefit from better quality (like PVRTC)
   {
      if(mip_maps<0)mip_maps=((src.mipMaps()==1) ? 1 : 0); // source will have now only one mip-map so we can't use "-1", auto-detect instead
      if(mode    <0)mode    =src.mode();                   // source will now be as IMAGE_SOFT      so we can't use "-1", auto-detect instead
      if(src.copyTry(temp, -1, -1, -1, IMAGE_R8G8B8, IMAGE_SOFT, 1))src=&temp;
   }

   IMAGE_TYPE    dest_type;
   if(force_type)dest_type=IMAGE_TYPE(*force_type);else
   if(type==ElmImage.ALPHA)dest_type=IMAGE_A8;else
   if(type==ElmImage.FULL )dest_type=(has_color ? IMAGE_R8G8B8A8 : has_alpha ? IMAGE_L8A8 : IMAGE_L8);else
                           ImageProps(*src, null, &dest_type, (ignore_alpha ? IGNORE_ALPHA : 0) | ((type==ElmImage.COMPRESSED2) ? FORCE_HQ : 0));

   if(src.type()==IMAGE_L8 && dest_type==IMAGE_A8
   || src.type()==IMAGE_A8 && dest_type==IMAGE_L8)
   {
      Image temp2; if(temp2.createSoftTry(src.w(), src.h(), src.d(), dest_type) && src.lockRead())
      {
         REPD(z, temp2.d())
         REPD(y, temp2.h())
         REPD(x, temp2.w())temp2.pixel3D(x, y, z, src.pixel3D(x, y, z));
         src.unlock();
         Swap(temp, temp2); src=&temp;
      }
   }
   return src.copyTry(game, size.x, size.y, size.z, dest_type, mode, mip_maps, FILTER_BEST, true, true);
}
bool EditToGameImage(Image &edit, Image &game, C ElmImage &data, C int *force_type=null)
{
   return EditToGameImage(edit, game, data.pow2(), data.alphaLum(), data.type, data.mode, data.mipMaps() ? 0 : 1, data.hasColor(), data.hasAlpha3(), data.ignoreAlpha(), data.size, force_type);
}
/******************************************************************************/
void DrawPanelImage(C PanelImage &pi, C Rect &rect, bool draw_lines=false)
{
   Vec2 size=rect.size()/5;
   Rect r=rect; r.extend(-size/3);
   Rect_LU lu(r.lu(), size            ); Rect_RU ru(r.ru(), size.x*3, size.y  );
   Rect_LD ld(r.ld(), size.x, size.y*3); Rect_RD rd(r.rd(), size.x*3, size.y*3);
   pi.draw(lu); pi.draw(ru);
   pi.draw(ld); pi.draw(rd);
   if(draw_lines)
   {
      pi.drawScaledLines(RED, lu); pi.drawScaledLines(RED, ru);
      pi.drawScaledLines(RED, ld); pi.drawScaledLines(RED, rd);
      lu.draw(TURQ, false); ru.draw(TURQ, false);
      ld.draw(TURQ, false); rd.draw(TURQ, false);
   }
}
/******************************************************************************/
bool UpdateMtrlTex(C Image &src, Image &dest)
{
   Image temp; if(src.copyTry(temp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))
   {
      // old: r=spec, g=NrmY, b=alpha, a=NrmX
      // new: r=NrmX, g=NrmY, b=spec , a=alpha
      REPD(y, temp.h())
      REPD(x, temp.w())
      {
         Color c=temp.color(x, y);
         c.set(c.a, c.g, c.r, c.b);
         temp.color(x, y, c);
      }
      return temp.copyTry(dest, -1, -1, -1, src.type()==IMAGE_BC3 ? IMAGE_BC7 : src.type(), src.mode(), src.mipMaps(), FILTER_BEST, true, false, false, true);
   }
   return false;
}
/******************************************************************************/
void AdjustMaterialParams(EditMaterial &edit, Material &game, uint old_base_tex, uint new_base_tex)
{
   TimeStamp time; time.getUTC();
   game._adjustParams(old_base_tex, new_base_tex);
   SyncByValue     (edit.      tech_time, time, edit.tech    , game.technique);
   SyncByValueEqual(edit.     color_time, time, edit.color.w , game.color.w  );
   SyncByValueEqual(edit.rough_bump_time, time, edit.bump    , game.bump     );
   SyncByValueEqual(edit.rough_bump_time, time, edit.rough   , game.rough    );
   SyncByValueEqual(edit.      spec_time, time, edit.specular, game.specular );
   SyncByValueEqual(edit.      glow_time, time, edit.glow    , game.glow     );
}
/******************************************************************************/
bool ImportImage(Image &image, C Str &name, int type=-1, int mode=-1, int mip_maps=-1, bool decompress=false)
{
   if(image.ImportTry(name, type, mode, mip_maps))
   {
      if(image.compressed() && decompress && !image.copyTry(image, -1, -1, -1, IMAGE_R8G8B8A8))return false;
      return true;
   }
 /*if(name.is())
   {
      File f, dec; if(f.readTry(name+".cmpr"))if(Decompress(f, dec.writeMem()))
      {
         dec.pos(0); if(image.ImportTry(dec, type, mode, mip_maps))return true;
      }
   }*/
   return false;
}
/******************************************************************************/
// TEXT
/******************************************************************************/
bool ValidChar(char c) {return c>=32 && c<128;}
bool ValidText(C Str &text, int min=1, int max=-1)
{
   if(text.length()>=min && (text.length()<=max || max<0))
   {
      REPA(text)if(!ValidChar(text[i]))return false;
      return true;
   }
   return false;
}
bool ValidFileName(C Str &name) {return name.length()>=1 && CleanFileName(name)==name;}
bool ValidFileNameForUpload(C Str &name, int max=128)
{
   if(ValidFileName(name) && ValidText(name, 1, max))
   {
      REPA(name)if(name[i]=='@' || name[i]=='~')return false; // because of "@new" and "~" is special char on unix ?
      return true;
   }
   return false;
}
bool ValidPass(C Str &pass) {return ValidText(pass, 4, 16) && !HasUnicode(pass);}
bool ValidEnum(C Str &name)
{
   if(!name.is())return false;
   FREPA(name)
   {
      uint flag=CharFlag(name[i]);
      bool ok=((flag&(CHARF_ALPHA|CHARF_UNDER)) || (i && (flag&CHARF_DIG))); // digit cannot be used as first character
      if(! ok)return false;
   }
   return true;
}
bool ValidSupport(C Str &support)
{
   return !support.is() || ValidEmail(support) || ValidURL(support);
}
bool ValidVideo(C Str &video)
{
   return !video.is() || (ValidURL(video) && (StartsPath(video, "http://www.youtube.com/embed") || StartsPath(video, "https://www.youtube.com/embed")));
}
Str YouTubeEmbedToFull(C Str &video) {return Replace(video, "/embed/", "/watch?v=");}
Str YouTubeFullToEmbed(C Str &video) {return Replace(video, "/watch?v=", "/embed/");}
UID PassToMD5(C Str &pass) {return MD5Mem((Str8)CaseDown(pass), pass.length());}
Str NameToEnum(C Str &name)
{
   Str e; FREPA(name)
   {
      char c=name[i];
      if(c==' ')c='_';
      if(c>='0' && c<='9'
      || c>='a' && c<='z'
      || c>='A' && c<='Z'
      || c=='_')e+=c;
   }
   return e;
}
Str TimeAgo(C DateTime &date) {DateTime now; now.getUTC(); return TimeText(now-date, TIME_NAME_MED)+" ago";}
char CountS(int n) {return (n==1) ? '\0' : 's';}
Str  Plural(Str name) // convert to plural name
{
   bool case_up=Equal(CaseUp(name), name, true);
   char last   =CaseDown(name.last());
   if(name=="child"                      )name+="ren"               ;else // child  -> children
   if(name=="potato"                     )name+="es"                ;else // potato -> potatoes
   if(name=="hero"                       )name+="es"                ;else // hero   -> heroes
   if(name=="mouse"                      )name.remove(1, 4)+="ice"  ;else // mouse  -> mice
   if(name=="man"                        )name.remove(1, 2)+="en"   ;else // man    -> men
   if(name=="woman"                      )name.remove(3, 2)+="en"   ;else // woman  -> women
   if(name=="goose"                      )name.remove(1, 4)+="eese" ;else // goose  -> geese
   if(name=="person"                     )name.remove(1, 5)+="eople";else // person -> people
   if(last=='y'                          )name.removeLast()+="ies"  ;else // body   -> bodies
   if(last=='x' || last=='h' || last=='s')name+="es"                ;else // box    -> boxes, mesh -> meshes, bus -> buses
   if(last=='f')
   {
      if(name!="dwarf" && name!="roof")name.removeLast()+="ves"; // leaf -> leaves, elf -> elves (dwarf -> dwarfs, roof -> roofs)
   }else
   if(Ends(name, "fe"))name.removeLast().removeLast()+="ves";else // life -> lives, knife -> knives
      name+='s';
   return case_up ? CaseUp(name) : name;
}
Str RemoveQuotes(Str s)
{
   if(s.last ()=='"')s.removeLast();
   if(s.first()=='"')s.remove(0);
   return s;
}
/******************************************************************************/
Str VecI2AsText(C VecI2 &v) // try to keep as one value if XY are the same
{
   Str s; s=v.x; if(v.y!=v.x)s+=S+","+v.y; 
   return s;
}
Vec2 TextVec2Ex(cchar *t)
{
   return Contains(t, ',') ? TextVec2(t) : Vec2(TextFlt(t));
}
Vec TextVecEx(cchar *t)
{
   return Contains(t, ',') ? TextVec(t) : Vec(TextFlt(t));
}
Str TextVecEx(C Vec &v, int precision=-3)
{
   return (Equal(v.x, v.y) && Equal(v.x, v.z)) ? TextReal(v.x, precision) : v.asText(precision);
}
/******************************************************************************/
Str RelativePath  (C Str &path) {return SkipStartPath(path, GetPath(App.exe()));}
Str EditToGamePath(  Str  path)
{
   Str out;
   for(path.tailSlash(false); ; )
   {
      Str base=GetBase(path); path=GetPath(path);
      if(!base.is()   )return path.tailSlash(true)+out; // needed for "/xxx" unix style paths
      if( base=="Edit")return path.tailSlash(true)+"Game\\"+out; // replace "edit" with "game"
      out=(out.is() ? base.tailSlash(true)+out : base);
   }
}
/******************************************************************************/
cchar8 *FormatSuffixes[]=
{
   "_BC1",
   "_BC3", // for Web
   "_ETC1",
   "_ETC2",
   "_ETC2_A8",
   "_PVRTC1_2",
   "_PVRTC1_4",
   "_SIMPLE", // used for simplified Materials
};  int FormatSuffixElms=Elms(FormatSuffixes);
cchar8* FormatSuffixSimple() {return "_SIMPLE";}
cchar8* FormatSuffix(IMAGE_TYPE type)
{
   switch(type)
   {
      default: return null;

      case IMAGE_BC1: return "_BC1";
      case IMAGE_BC3: return "_BC3";

      case IMAGE_ETC1   : return "_ETC1";
      case IMAGE_ETC2   : return "_ETC2";
      case IMAGE_ETC2_A8: return "_ETC2_A8";

      case IMAGE_PVRTC1_2: return "_PVRTC1_2";
      case IMAGE_PVRTC1_4: return "_PVRTC1_4";
   }
}
Str8 ImageDownSizeSuffix(int size)
{
   if(size>0 && size<INT_MAX)return S+"_"+size;
   return S;
}
/******************************************************************************/
bool     MonoTransform(C TextParam &p   ) {return p.name=="grey" || p.name=="greyPhoto" || p.name=="bump" || (p.name=="channel" && p.value.length()==1);}
bool   ResizeTransform(C Str       &name) {return name=="resize" || name=="resizeWrap" || name=="resizeClamp" || name=="resizeLinear" || name=="resizeCubic" || name=="resizeNoStretch" || name=="maxSize";}
bool       UVTransform(C Str       &name) {return ResizeTransform(name) || name=="crop" || name=="swapXY" || name=="mirrorX" || name=="mirrorY";}
bool  NonMonoTransform(C Str       &name) {return UVTransform(name) || name=="blur" || name=="normalize";} // if can change a mono image to non-mono, this is NOT the same as "!MonoTransform"
bool HighPrecTransform(C Str       &name)
{
   return ResizeTransform(name)
       || name=="mulRGB" || name=="addRGB" || name=="mulAddRGB" || name=="addMulRGB" || name=="mulA"
       || name=="mulRGBS" || name=="mulRGBH" || name=="mulRGBHS"
       || name=="normalize"
       || name=="scale" || name=="scaleXY"
       || name=="lerpRGB" || name=="ilerpRGB"
       || name=="blur"
       || name=="bump"
       || name=="contrast" || name=="contrastLum" || name=="contrastAlphaWeight" || name=="contrastLumAlphaWeight"
       || name=="brightness" || name=="brightnessLum"
       || name=="gamma" || name=="gammaLum"
       || name=="greyPhoto"
       || name=="avgLum" || name=="medLum" || name=="avgContrastLum" || name=="medContrastLum"
       || name=="avgHue" || name=="medHue" || name=="addHue" || name=="setHue" || name=="contrastHue" || name=="contrastHueAlphaWeight" || name=="contrastHuePow"
       || name=="lerpHue" || name=="lerpHueSat" || name=="rollHue" || name=="rollHueSat" || name=="lerpHuePhoto" || name=="lerpHueSatPhoto" || name=="rollHuePhoto" || name=="rollHueSatPhoto"
       || name=="addSat" || name=="mulSat" || name=="mulSatPhoto" || name=="avgSat" || name=="contrastSat" || name=="contrastSatAlphaWeight"
       || name=="addHueSat" || name=="setHueSat" || name=="setHueSatPhoto"
       || name=="mulSatH" || name=="mulSatHS" || name=="mulSatHPhoto" || name=="mulSatHSPhoto";
}

TextParam* FindTransform(MemPtr<Edit.FileParams> files, C Str &name)
{
   int files_with_names=0; REPA(files)if(files[i].name.is())files_with_names++; // count how many files have names(images) and aren't just transforms
   REPA(files)
   {
      Edit.FileParams &file=files[i];
      if(files_with_names<=1 || !file.name.is())REPA(file.params)
      {
         TextParam &p=file.params[i]; if(p.name==name && !Contains(p.value, '@'))return &p;
      }
   }
   return null;
}
void DelTransform(MemPtr<Edit.FileParams> files, C Str &name)
{
   int files_with_names=0; REPA(files)if(files[i].name.is())files_with_names++; // count how many files have names(images) and aren't just transforms
   REPA(files)
   {
      Edit.FileParams &file=files[i];
      if(files_with_names<=1 || !file.name.is())REPA(file.params)
      {
         TextParam &p=file.params[i]; if(p.name==name && !Contains(p.value, '@'))
         {
            file.params.remove(i, true);
            return;
         }
      }
   }
}
void SetTransform(MemPtr<Edit.FileParams> files, C Str &name, C Str &value=S)
{
   if(files.elms())
   {
      int files_with_names=0; REPA(files)if(files[i].name.is())files_with_names++; // count how many files have names(images) and aren't just transforms
      if( files_with_names>1) // if we have more than one image, then we need to make sure that we add the parameter not to one of the images, but as last file that has no name specified
         if(files.last().name.is()) // if the last file has a name specified
            files.New(); // create an empty file with no name
      Edit.FileParams &file=files.last(); // set param to the last file
      TextParam *p;
      REPA(file.params)
      {
         p=&file.params[i]; if(p.name==name && !Contains(p.value, '@'))goto found;
      }
      p=&file.params.New().setName(name);
   found:
      p.setValue(value);
   }
}
bool ForcesMono(C Str &file)
{
   Mems<Edit.FileParams> files=Edit.FileParams.Decode(file);
   if(files.elms())
   {
    C Edit.FileParams &file=files.last();
      if(!file.name.is() || files.elms()==1)REPA(file.params) // go from end
      {
       C TextParam &param=file.params[i];
         if(    MonoTransform(param     ))return true;
         if(!NonMonoTransform(param.name))break;
      }
   }
   return false;
}
void SetTransform(Str &file, C Str &name, C Str &value=S)
{
   Mems<Edit.FileParams> files=Edit.FileParams.Decode(file);
   SetTransform(files, name, value);
   file=Edit.FileParams.Encode(files);
}
Str BumpFromColTransform(C Str &color_map, int blur)
{
   Mems<Edit.FileParams> files=Edit.FileParams.Decode(color_map);
   if(files.elms()==1)
   {
      Edit.FileParams &file=files[0];
      REPA(file.params)
      {
         TextParam &par=file.params[i]; if(par.name!="crop" && par.name!="swapXY" && par.name!="mirrorX" && par.name!="mirrorY")file.params.remove(i, true);
      }
   }
   SetTransform(files, "bump", (blur<0) ? S : S+blur);
   return Edit.FileParams.Encode(files);
}
/******************************************************************************/
SOUND_CODEC TextSoundCodec(C Str &t)
{
   if(t=="raw"
   || t=="wav"
   || t=="uncompressed")return SOUND_WAV;
   if(t=="vorbis"      )return SOUND_SND_VORBIS;
   if(t=="opus"        )return SOUND_SND_OPUS;
                        return SOUND_NONE;
}
/******************************************************************************/
// MESH
/******************************************************************************/
int VisibleVtxs      (C MeshLod &mesh) {int num =0; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))num +=mesh.parts[i].vtxs           (); return num ;}
int VisibleTris      (C MeshLod &mesh) {int num =0; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))num +=mesh.parts[i].tris           (); return num ;}
int VisibleTrisTotal (C MeshLod &mesh) {int num =0; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))num +=mesh.parts[i].trisTotal      (); return num ;}
int VisibleQuads     (C MeshLod &mesh) {int num =0; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))num +=mesh.parts[i].quads          (); return num ;}
int VisibleSize      (C MeshLod &mesh) {int size=0; REPA(mesh)if(!(mesh.parts[i].part_flag&MSHP_HIDDEN))size+=mesh.parts[i].render.memUsage(); return size;}
flt VisibleLodQuality(C Mesh    &mesh, int lod_index)
{
   Clamp(lod_index, 0, mesh.lods());
 C MeshLod &base=mesh,
           &lod =mesh.lod(lod_index);
   Int      v   =VisibleVtxs     (base),
            f   =VisibleTrisTotal(base);
   return Avg(v ? flt(VisibleVtxs     (lod))/v : 1,
              f ? flt(VisibleTrisTotal(lod))/f : 1);
}
/******************************************************************************/
void KeepParams(C Mesh &src, Mesh &dest)
{
   REPAD(d, dest)
   {
      MeshPart &dest_part=dest.parts[d];
      int        src_part_i=-1;
      REPAD(s, src)
      {
       C MeshPart &src_part=src.parts[s];
         if(Equal(src_part.name, dest_part.name)) // if same name
            if(src_part_i<0 || Abs(s-d)<Abs(src_part_i-d))src_part_i=s; // if new index is closer to original index
      }
      if(InRange(src_part_i, src))
      {
       C MeshPart &src_part=src.parts[src_part_i];
         dest_part.part_flag=src_part.part_flag;
         dest_part.drawGroup(src_part.drawGroup(), src.drawGroupEnum());
         dest_part.variations(src_part.variations());
         REP(dest_part.variations())if(i)dest_part.variation(i, src_part.variation(i));
      }
   }
   dest.drawGroupEnum(src.drawGroupEnum()); // keep the same draw group enum
}
void EditToGameMesh(C Mesh &edit, Mesh &game, Skeleton *skel, Enum *draw_group, C Matrix *matrix)
{
   game.create(edit, GameMeshFlagAnd);
   // cleanup mesh
   REPD(l, game.lods()) // have to go from end because we're removing LOD's
   {
      MeshLod &lod=game.lod(l);
      // remove LOD's
      if(NegativeSB(lod.dist2) // negative distance (marked as disabled)
      || InRange(l+1, game.lods()) && lod.dist2>=game.lod(l+1).dist2) // distance is higher than the next one (have to check next one and not previous one, because we need to delete those with negative distance first)
      {
      remove_lod:
         game.removeLod(l);
      }else
      {
         // remove hidden mesh parts
         REPA(lod)if(lod.parts[i].part_flag&MSHP_HIDDEN)lod.parts.remove(i);
         if(!lod.parts.elms())goto remove_lod;
      }
   }
   game.joinAll(true, true, false, MeshJoinAllTestVtxFlag, -1); // disable vtx weld, because: 1) mesh isn't scaled/transformed yet 2) it would be performed only if some parts were merged 3) there are no tangents yet. Instead let's always do it manually
   if(matrix)game.transform(*matrix); // transform before welding
   game.setAutoTanBin() // calculate tangents before welding
       .weldVtx(VTX_ALL, EPS, EPS_COL8_COS, -1).skeleton(skel).drawGroupEnum(draw_group).setBox(); // use 8-bit for vtx normals because they can't handle more anyway, always recalculate box because of transform and welding
   game.setRender().delBase();
}
/******************************************************************************/
bool HasMaterial(C MeshPart &part, C MaterialPtr &material)
{
   REP(part.variations())if(part.variation(i)==material)return true;
   return false;
}
/******************************************************************************/
bool FixVtxNrm(MeshBase &base)
{
   bool ok=false;
   MeshBase temp(base, VTX_POS|FACE_IND); temp.setNormals();
   if(base.vtxs()==temp.vtxs() && temp.vtx.nrm()) // safety checks
   {
      ok=true;
      REPA(base.vtx)
      {
         Vec &nrm=base.vtx.nrm(i); if(!nrm.any())
         {
            nrm=temp.vtx.nrm(i); if(!nrm.any())ok=false;
         }
      }
   }
   return ok;
}
void FixMesh(Mesh &mesh)
{
   mesh.setBase(true).delRender() // create base if empty
       .material(null).variations(0) // clear any existing materials, they will be set later according to 'mtrls'
       .skeleton(null).drawGroupEnum(null) // clear 'skeleton', clear 'drawGroupEnum'
       .removeUnusedVtxs(); // remove unused vertexes

   // check if some vtx normals are wrong
   REP(mesh.lods())
   {
      MeshLod &lod=mesh.lod(i); REPA(lod)
      {
         MeshPart &part=lod.parts[i];
         MeshBase &base=part.base;
         if(base.vtx.nrm())REPA(base.vtx)if(!base.vtx.nrm(i).any()) // all zero
         {
            if(!FixVtxNrm(base)) // if didn't fix yet, then it's possible that vtx shares only co-planar faces
            {
               base.explodeVtxs();
               FixVtxNrm(base);
               base.weldVtx(VTX_ALL, EPSD, EPS_COL_COS, -1); // use small epsilon in case mesh is scaled down, do not remove degenerate faces because they're not needed because we're doing this only because of 'explodeVtxs'
            }
            break;
         }
      }
   }
}
bool SamePartInAllLods(C Mesh &mesh, int part)
{
   if(InRange(part, mesh.parts))
   {
      cchar8 *name=mesh.parts[part].name;
      for(Int i=mesh.lods(); --i>=1; )
      {
       C MeshLod &lod=mesh.lod(i);
         if(!InRange(part, lod) || !Equal(name, lod.parts[part].name))return false;
      }
      return true;
   }
   return false;
}
void SetDrawGroup(Mesh &mesh, MeshLod &lod, int part, int group, Enum *draw_group_enum)
{
   if(SamePartInAllLods(mesh, part))
   {
      REP(mesh.lods())
      {
         MeshLod &lod=mesh.lod(i);
         if(InRange(part, lod))lod.parts[part].drawGroup(group, draw_group_enum);
      }
   }else
   {
      if(InRange(part, lod))lod.parts[part].drawGroup(group, draw_group_enum);
   }
}
/******************************************************************************/
// SKELETON
/******************************************************************************/
Str BoneNeutralName(C Str &name)
{
   Str n=Replace(name, "right", CharAlpha);
       n=Replace(n   , "left" , CharAlpha);
       n.replace('r', CharBeta).replace('l', CharBeta).replace('R', CharBeta).replace('L', CharBeta);
   return n;
}
/******************************************************************************/
// OBJECT
/******************************************************************************/
// following functions are used to determine whether object should override mesh/phys
bool OverrideMeshSkel(C Mesh *mesh, C Skeleton *skel) {return (mesh && mesh->is()) || (skel && skel->is());}
bool OverridePhys    (C PhysBody *body              ) {return (body && body->is());}

int CompareObj(C Game.Area.Data.AreaObj &a, C Game.Area.Data.AreaObj &b) // this function is used for sorting object before they're saved into game area
{
   if(int c=Compare(a.mesh().id()      , b.mesh().id()      ))return c; // first compare by mesh
   if(int c=Compare(a.meshVariationID(), b.meshVariationID()))return c; // then  compare by mesh variation
   if(int c=Compare(a.matrix.pos       , b.matrix.pos       ))return c; // last  compare by position
   return 0;
}
/******************************************************************************/
// ANIMATION
/******************************************************************************/
void SetRootMoveRot(Animation &anim, C Vec *root_move, C Vec *root_rot)
{
   if(root_rot)
   {
      const int precision=4; // number of keyframes per 90 deg, can be modified, this is needed because rotation interpolation is done by interpolating axis vectors, and few samples are needed to get smooth results
      int num=(Equal(*root_rot, VecZero) ? Equal(anim.rootStart().angle(), 0) ? 0 : 1 : 1+Max(1, Round(root_rot.length()*(precision/PI_2))));
      anim.keys.orns.setNum(num);
      if(num)
      {
         OrientD orn=anim.rootStart();
         anim.keys.orns[0].time=0;
         anim.keys.orns[0].orn =orn;
         if(num>=2)
         {
            num--;
            VecD axis=*root_rot; dbl angle=axis.normalize(); MatrixD3 rot; rot.setRotate(axis, angle/num);
            for(int i=1; i<=num; i++)
            {
               orn.mul(rot, true);
               anim.keys.orns[i].time=flt(i)/num*anim.length();
               anim.keys.orns[i].orn =orn;
            }
         }
      }
   }
   if(root_move)
   {
      const int precision=10; // number of keyframes per meter, can be modified
      bool no_rot=(!root_rot || anim.keys.orns.elms()<=1);
      int  num=(Equal(*root_move, VecZero) ? Equal(anim.rootStart().pos, VecZero) ? 0 : 1 : no_rot ? 2 : 1+Max(1, Round(root_move.length()*precision)));
      anim.keys.poss.setNum(num);
      if(num)
      {
         anim.keys.poss[0].time=0;
         anim.keys.poss[0].pos =anim.rootStart().pos;
         if(num>=2)
         {
            if(no_rot)
            {
               anim.keys.poss[1].time=anim.length();
               anim.keys.poss[1].pos =anim.rootStart().pos+*root_move;
            }else
            {
               num--;
               VecD pos=anim.rootStart().pos, dir=*root_move/num, axis=*root_rot; dbl angle=axis.normalize(); MatrixD3 rot; rot.setRotate(axis, angle/num);
               for(int i=1; i<=num; i++)
               {
                  dir*=rot; pos+=dir;
                  anim.keys.poss[i].time=flt(i)/num*anim.length();
                  anim.keys.poss[i].pos =pos;
               }
            }
         }
      }
   }
   if(root_move || root_rot)
   {
      anim.keys.setTangents(anim.loop(), anim.length());
      anim.setRootMatrix();
   }
}
/******************************************************************************/
// MATH
/******************************************************************************/
inline bool NegativeSB(flt  x) {return FlagTest  ((uint&)x, SIGN_BIT);} // have to work with SIGN_BIT for special case of -0
inline void      CHSSB(flt &x) {       FlagToggle((uint&)x, SIGN_BIT);} // have to work with SIGN_BIT for special case of -0
/******************************************************************************/
int UniquePairs(int elms) {return elms*(elms-1)/2;}
/******************************************************************************/
bool Distance2D(C Vec2 &point, C Edge &edge, flt &dist, flt min_length=0.025) // calculate 2D distance between 'point' and 'edge' projected to screen, true is returned if 'edge' is at least partially visible (not behind camera), 'dist' will contain distance between point and edge
{
   Edge e=edge;
   if(Clip(e, Plane(ActiveCam.matrix.pos + D.viewFrom()*ActiveCam.matrix.z, -ActiveCam.matrix.z)))
   {
      Edge2 e2(PosToScreen(e.p[0]), PosToScreen(e.p[1]));
      if(e2.length()<min_length)return false; // if edge is too short then skip it
      dist=Dist(point, e2);
      return true;
   }
   return false;
}
int MatrixAxis(C Vec2 &screen_pos, C Matrix &matrix)
{
   int axis=-1;
   flt d, dist=0;
   if(Distance2D(screen_pos, Edge(matrix.pos, matrix.pos+matrix.x), d))if(axis<0 || d<dist){dist=d; axis=0;}
   if(Distance2D(screen_pos, Edge(matrix.pos, matrix.pos+matrix.y), d))if(axis<0 || d<dist){dist=d; axis=1;}
   if(Distance2D(screen_pos, Edge(matrix.pos, matrix.pos+matrix.z), d))if(axis<0 || d<dist){dist=d; axis=2;}
   if(dist>0.05)axis=-1;
   return axis;
}
void MatrixAxis(Edit.Viewport4 &v4, C Matrix &matrix, int &axis, Vec *axis_vec=null)
{
   int editing=-1; REPA(MT)if(MT.b(i, MT.touch(i) ? 0 : 1) && v4.getView(MT.guiObj(i))){editing=i; break;}
   if( editing<0)
   {
      axis=-1;
      if(editing< 0)REPA(MT)if(!MT.touch(i) && v4.getView(MT.guiObj(i))){editing=i; break;} // get mouse
      if(editing>=0)if(Edit.Viewport4.View *view=v4.getView(MT.guiObj(editing)))
      {
         view.setViewportCamera();
         axis=MatrixAxis(MT.pos(editing), matrix);
      }
      if(axis_vec)switch(axis)
      {
         case  0: *axis_vec=!matrix.x; break;
         case  1: *axis_vec=!matrix.y; break;
         case  2: *axis_vec=!matrix.z; break;
         default:  axis_vec.zero()   ; break;
      }
   }
}
int GetNearestAxis(C Matrix &matrix, C Vec &dir)
{
   flt dx=Abs(Dot(!matrix.x, dir)),
       dy=Abs(Dot(!matrix.y, dir)),
       dz=Abs(Dot(!matrix.z, dir));
   return MaxI(dx, dy, dz);
}
bool UniformScale(C Matrix3 &m) {return UniformScale(m.scale());}
bool UniformScale(C Vec     &s)
{
   return Equal(s.x/s.y, 1)
       && Equal(s.x/s.z, 1);
}
/******************************************************************************/
flt CamMoveScale(bool perspective=true    ) {return perspective ? ActiveCam.dist*Tan(D.viewFov()/2) : D.viewFov();}
Vec2   MoveScale(Edit.Viewport4.View &view) {return Vec2(D.w()*2, D.h()*2)/view.viewport.size();}

flt AlignDirToCamEx(C Vec &dir, C Vec2 &delta, C Vec &cam_x=ActiveCam.matrix.x, C Vec &cam_y=ActiveCam.matrix.y) {return (!Vec2(Dot(cam_x, dir), Dot(cam_y, dir)) * delta).sum();}
Vec AlignDirToCam  (C Vec &dir, C Vec2 &delta, C Vec &cam_x=ActiveCam.matrix.x, C Vec &cam_y=ActiveCam.matrix.y) {return !dir * AlignDirToCamEx(dir, delta, cam_x, cam_y);}
/******************************************************************************/
flt MatrixLength(C Vec &x, C Vec &y, C Vec &z, C Vec &dir) // matrix length along direction
{
   return Abs(Dot(x, dir))
         +Abs(Dot(y, dir))
         +Abs(Dot(z, dir));
}
/******************************************************************************/
void Include(RectI &rect,           C VecI2 &x) {if(rect.valid())rect|=x;else          rect=x; }
void Include(RectI &rect,           C RectI &x) {if(rect.valid())rect|=x;else          rect=x; }
void Include(Rect  &rect,           C Rect  &x) {if(rect.valid())rect|=x;else          rect=x; }
void Include(Rect  &rect, bool &is, C Vec2  &x) {if(is          )rect|=x;else{is=true; rect=x;}}
void Include(Rect  &rect, bool &is, C Rect  &x) {if(is          )rect|=x;else{is=true; rect=x;}}
void Include(Box   &box , bool &is, C Vec   &v) {if(is          )box |=v;else{is=true; box =v;}}
void Include(Box   &box , bool &is, C Box   &b) {if(is          )box |=b;else{is=true; box =b;}}
/******************************************************************************/
void DrawMatrix(C Matrix &matrix, int axis)
{
   matrix.draw(); switch(axis)
   {
      case 0: DrawArrow2(RED  , matrix.pos, matrix.pos+matrix.x, 0.005); break;
      case 1: DrawArrow2(GREEN, matrix.pos, matrix.pos+matrix.y, 0.005); break;
      case 2: DrawArrow2(BLUE , matrix.pos, matrix.pos+matrix.z, 0.005); break;
   }
}
/******************************************************************************/
// MISC
/******************************************************************************/
void Hide(GuiObj &go) {go.hide();}
/******************************************************************************/
Rect GetRect(C Rect &rect, Memt<Rect> &rects) // !! this will modify 'rects' !!
{
   for(flt x=D.w(); ; ) // start with right side
   {
      for(flt y=D.h(); ; ) // start from top
      {
         Rect temp=rect; temp+=Vec2(x, y)-temp.ru(); // move to test position

         if(!rects.elms())return temp;

         if(temp.min.y<-D.h())break; // we're outside the screen

         bool cuts=false;
         REPA(rects)if(Cuts(temp, rects[i]))
         {
            MIN(y, rects[i].min.y-EPS);
            cuts=true;
         }
         if(!cuts)return temp;
      }

      // find the maximum out of rectangles min.x and remove those rectangles
      int found=-1; REPA(rects)if(found==-1 || rects[i].min.x>x){found=i; x=rects[i].min.x;}
                    REPA(rects)if(rects[i].min.x>=x-EPS)rects.remove(i);
   }
}
/******************************************************************************/
void Include(MemPtr<UID> ids, C UID &id)
{
   if(id.valid())ids.binaryInclude(id, Compare);
}
/******************************************************************************/
bool Same(C Memc<UID> &a, C Memc<UID> &b)
{
   if(a.elms()!=b.elms())return false;
   REPA(a)if(a[i]!=b[i])return false;
   return true;
}
bool Same(C Memc<ObjData> &a, C Memc<ObjData> &b)
{
   if(a.elms()!=b.elms())return false;
   REPA(a)
   {
    C ObjData &oa=a[i];
      REPA(b)
      {
       C ObjData &ob=b[i];
         if(oa.id==ob.id)if(oa.equal(ob))goto oa_equal;else break;
      }
      return false; // not found or not equal
      oa_equal:;
   }
   return true;
}
void GetNewer(C Memc<ObjData> &a, C Memc<ObjData> &b, Memc<UID> &newer) // get id's of 'a' objects that are newer than 'b'
{
   REPA(a)
   {
    C ObjData &oa=a[i], *ob=null;
      REPA(b)if(b[i].id==oa.id){ob=&b[i]; break;}
      if(!ob || oa.newer(*ob))newer.add(oa.id);
   }
}
/******************************************************************************/
bool EmbedObject(C Box &obj_box, C VecI2 &area_xy, flt area_size)
{
   return obj_box.min.x/area_size<area_xy.x-0.5 || obj_box.max.x/area_size>area_xy.x+1.5
       || obj_box.min.z/area_size<area_xy.y-0.5 || obj_box.max.z/area_size>area_xy.y+1.5;
}
/******************************************************************************/
bool SameOS(OS_VER a, OS_VER b)
{
   return OSWindows(a) && OSWindows(b)
       || OSMac    (a) && OSMac    (b)
       || OSLinux  (a) && OSLinux  (b)
       || OSAndroid(a) && OSAndroid(b)
       || OSiOS    (a) && OSiOS    (b);
}
/******************************************************************************/
UID GetFileNameID(Str name)
{
   for(; name.is(); name=GetPath(name)){UID id=FileNameID(name); if(id.valid())return id;}
   return UIDZero;
}
UID AsID(C Elm *elm) {return elm ? elm.id : UIDZero;}
/******************************************************************************/
void SetPath(WindowIO &win_io, C Str &path, bool clear=false)
{
   Mems<Edit.FileParams> fps=Edit.FileParams.Decode(path); if(fps.elms()==1)
   {
      Str first=FFirstUp(fps[0].name);
      if(FileInfoSystem(first).type==FSTD_FILE)first=GetPath(first);
      fps[0].name=SkipStartPath(fps[0].name, first);
      win_io.path(S, first).textline.set(fps[0].encode()); return;
   }else
   if(fps.elms()>1)
   {
      win_io.path(S).textline.set(path); return;
   }
   if(clear)win_io.path(S).textline.clear();
}
/******************************************************************************/
bool ParamTypeID        (PARAM_TYPE type           ) {return type==PARAM_ID || type==PARAM_ID_ARRAY;}
bool ParamTypeCompatible(PARAM_TYPE a, PARAM_TYPE b) {return a==b || (ParamTypeID(a) && ParamTypeID(b));}
bool ParamCompatible    (C Param   &a, C Param   &b) {return a.name==b.name && ParamTypeCompatible(a.type, b.type);}
/******************************************************************************/
class Rename
{
   Str src, dest;

   Rename& set(C Str &src, C Str &dest) {T.src=src; T.dest=dest; return T;}
   
   bool operator==(C Rename &rename)C {return Equal(src, rename.src, true) && Equal(dest, rename.dest, true);}
   bool operator!=(C Rename &rename)C {return !(T==rename);}
   
   /*enum CHECK
   {
      SAME,
      REVERSE,
      REQUIRED,
      UNKNOWN,
   }
   CHECK check(C Rename &rename)C
   {
      bool src_equal=Equal( src, rename. src, true),
          dest_equal=Equal(dest, rename.dest, true);
      if(src_equal && dest_equal)return SAME; // this is the same change

      bool src_equal_dest=Equal( src, rename.dest, true),
          dest_equal_src =Equal(dest, rename.src , true);
      if(src_equal_dest && dest_equal_src)return REVERSE; // this is a reverse change

      return (src_equal || dest_equal || src_equal_dest || dest_equal_src) ? REQUIRED : UNKNOWN; // if any of the names is used, then it is required
   }*/
}
/*void Add(MemPtr<Rename> diff, C Rename &rename, bool optimize=true)
{
   if(optimize)REPA(diff)switch(diff[i].check(rename)) // need to go from the end
   {
      case Rename.REQUIRED: goto add; // we already know that this is required, so skip checking
      case Rename.SAME    :                       return; // no need to apply the same change twice
      case Rename.REVERSE : diff.remove(i, true); return;
    //case Rename.UNKNOWN : break; // keep on checking
   }
add:
   diff.add(rename);
}
void AddReverse(MemPtr<Rename> diff, C Rename &rename, bool optimize=true)
{
   if(optimize)REPA(diff)switch(diff[i].check(rename)) // need to go from the end
   {
      case Rename.REQUIRED: goto add; // we already know that this is required, so skip checking
      case Rename.SAME    : diff.remove(i, true); return; //                                        !! HERE SAME AND REVERSE ARE SWITCHED !!
      case Rename.REVERSE :                       return; // no need to apply the same change twice !! HERE SAME AND REVERSE ARE SWITCHED !!
    //case Rename.UNKNOWN : break; // keep on checking
   }
add:
   diff.New().set(rename.dest, rename.src); // we're reversing so we need to replace dest with src
}
void Diff(MemPtr<Rename> diff, C MemPtr<Rename> &current, C MemPtr<Rename> &desired, bool optimize=true)
{
   diff.clear();
   int min=Min(current.elms(), desired.elms()), equal=0; for(; equal<min; equal++)if(current[equal]!=desired[equal])break; // calculate amount of equal changes

   // reverse 'current'
   for(int i=current.elms(); --i>=equal; )AddReverse(diff, current[i], optimize); // need to go from back

   // apply 'desired'
   for(int i=equal; i<desired.elms(); i++)Add(diff, desired[i], optimize); // need to go from start
}
/******************************************************************************/
enum UNIT_TYPE
{
   UNIT_DEFAULT,
   UNIT_PIXEL  ,
   UNIT_PERCENT, // 1/100
   UNIT_PERMIL , // 1/1000
}
UNIT_TYPE UnitType(C Str &s)
{
   if(s=="px"                 )return UNIT_PIXEL;
   if(s=="pc" || s=='%'       )return UNIT_PERCENT;
   if(s=="pm" || s==CharPermil)return UNIT_PERMIL;
   return UNIT_DEFAULT;
}
UNIT_TYPE GetUnitType(C Str &s)
{
   if(s.is())
   {
      const uint flag_and=CHARF_DIG10|CHARF_SIGN|CHARF_ALPHA|CHARF_UNDER|CHARF_SPACE;
      uint flag=CharFlag(s.last())&flag_and;
      int  pos =s.length()-1; for(; pos>0 && (CharFlag(s[pos-1])&flag_and)==flag; pos--);
      return UnitType(s()+pos);
   }
   return UNIT_DEFAULT;
}
flt ConvertUnitType(flt value, flt full, UNIT_TYPE unit)
{
   switch(unit)
   {
      default          : return value;
      case UNIT_PERCENT: return value/ 100*full;
      case UNIT_PERMIL : return value/1000*full;
   }
}
/******************************************************************************/
// GUI
/******************************************************************************/
Color BackgroundColor()
{
   return Gui.backgroundColor();
}
Color BackgroundColorLight()
{
   Color col=BackgroundColor();
   byte  lum=col.lum(), add=44; Color col_add(add); if(lum)col_add=ColorMul(col, flt(add)/lum); // set normalized color (col/col.lum)*add
   return ColorAdd(col, col_add);
}
Color GuiListTextColor()
{
   if(Gui.skin && Gui.skin->list.text_style)return Gui.skin->list.text_style->color;
   return BLACK;
}
const Color LitSelColor=RED, SelColor=YELLOW, LitColor=CYAN, DefColor=WHITE, InvalidColor=PURPLE;
Color GetLitSelCol(bool lit, bool sel, C Color &none=DefColor)
{
   if(lit && sel)return LitSelColor;
   if(       sel)return SelColor;
   if(lit       )return LitColor;
                 return none;
}
bool ErrorCopy(C Str &src, C Str &dest)
{
   Gui.msgBox(S, S+"Error copying\n\""+src+"\"\nto\n\""+dest+'"');
   return false;
}
bool ErrorRecycle(C Str &name)
{
   Gui.msgBox(S, S+"Error recycling\n\""+name+"\"");
   return false;
}
bool ErrorCreateDir(C Str &name)
{
   Gui.msgBox(S, S+"Error creating folder\n\""+name+"\"");
   return false;
}
bool RecycleLoud  (C Str &name            ) {return FRecycle   (name     ) ? true : ErrorRecycle  (name);}
bool CreateDirLoud(C Str &name            ) {return FCreateDirs(name     ) ? true : ErrorCreateDir(name);}
bool SafeCopyLoud (C Str &src, C Str &dest) {return SafeCopy   (src, dest) ? true : ErrorCopy     (src, dest);}
/******************************************************************************/
// SOUND
/******************************************************************************/
class BitRateQuality
{
   flt quality;
   int bit_rate;
}
const BitRateQuality BitRateQualities[]=
{
   {-0.2,  32*1000}, // aoTuV only
   {-0.1,  45*1000},
   { 0.0,  64*1000},
   { 0.1,  80*1000},
   { 0.2,  96*1000},
   { 0.3, 112*1000},
   { 0.4, 128*1000},
   { 0.5, 160*1000},
   { 0.6, 192*1000},
   { 0.7, 224*1000},
   { 0.8, 256*1000},
   { 0.9, 320*1000},
   { 1.0, 500*1000},
};
flt VorbisBitRateToQuality(int rel_bit_rate) // relative bit rate in bits per second (bit rate for 44.1kHz stereo)
{
   for(int i=1; i<Elms(BitRateQualities); i++)if(rel_bit_rate<=BitRateQualities[i].bit_rate)
   {
    C BitRateQuality &p=BitRateQualities[i-1],
                     &n=BitRateQualities[i  ];
      flt step=LerpR(p.bit_rate, n.bit_rate, rel_bit_rate);
      return   Lerp (p.quality , n.quality , step);
   }
   return 1;
}
/******************************************************************************/
// DEPRECATED
/******************************************************************************/
int DecIntV(File &f)
{
   Byte v; f>>v;
   Bool positive=((v>>6)&1);
   UInt u=(v&63);
   if(v&128)
   {
      f>>v; u|=((v&127)<<6);
      if(v&128)
      {
         f>>v; u|=((v&127)<<(6+7));
         if(v&128)
         {
            f>>v; u|=((v&127)<<(6+7+7));
            if(v&128)
            {
               f>>v; u|=(v<<(6+7+7+7));
            }
         }
      }
   }
   return positive ? u+1 : -Int(u);
}

void GetStr2(File &f, Str &s) {s=GetStr2(f);}
Str  GetStr2(File &f)
{
   Int length=DecIntV(f);
   if( length<0) // unicode
   {
      CHS(length); MIN(length, f.left()/2);
      Str s; s.reserve(length); REP(length){char c; f>>c; s+=c;} return s;
   }else
   if(length)
   {
      MIN(length, f.left());
      Str8 s; s.reserve(length); REP(length){char8 c; f>>c; s+=c;} return s;
   }
   return S;
}

void PutStr(File &f, C Str &s)
{
   uint length =s.length();
   bool unicode=HasUnicode(s);

   f.putUInt(unicode ? length^SIGN_BIT : length);
   if(length)
   {
      if(unicode){          f.putN(s(), length);}
      else       {Str8 t=s; f.putN(t(), length);}
   }
}
Str GetStr(File &f)
{
   uint length=f.getUInt();
   if(  length&SIGN_BIT) // unicode
   {
         length^=SIGN_BIT; MIN(length, f.left()/2);
      if(length){Str s; s.reserve(length); REP(length){char c; f>>c; s+=c;} return s;}
   }else
   {
      MIN(length, f.left());
      if (length){Str8 s; s.reserve(length); REP(length){char8 c; f>>c; s+=c;} return s;}
   }
   return S;
}
void GetStr(File &f, Str &s) {s=GetStr(f);}

<TYPE      > bool Save(File &f, C Memc<TYPE> &m              ) {  f.putInt(m.elms()); FREPA(m)if(!m[i].save(f      ))return false; return f.ok();}
<TYPE, USER> bool Save(File &f, C Memc<TYPE> &m, C USER &user) {  f.putInt(m.elms()); FREPA(m)if(!m[i].save(f, user))return false; return f.ok();}
<TYPE      > bool Save(File &f, C Memx<TYPE> &m              ) {  f.putInt(m.elms()); FREPA(m)if(!m[i].save(f      ))return false; return f.ok();}
<TYPE      > bool Load(File &f,   Memc<TYPE> &m              ) {m.setNum(f.getInt()); FREPA(m)if(!m[i].load(f      ))goto   error; if(f.ok())return true; error: m.clear(); return false;}
<TYPE, USER> bool Load(File &f,   Memc<TYPE> &m, C USER &user) {m.setNum(f.getInt()); FREPA(m)if(!m[i].load(f, user))goto   error; if(f.ok())return true; error: m.clear(); return false;}
<TYPE      > bool Load(File &f,   Memx<TYPE> &m              ) {m.setNum(f.getInt()); FREPA(m)if(!m[i].load(f      ))goto   error; if(f.ok())return true; error: m.clear(); return false;}

Mems<Edit.FileParams> _DecodeFileParams(C Str &str)
{
   Mems<Edit.FileParams> files; if(str.is())
   {
      Memc<Str> strs=Split(str, '|'); // get list of all files
      files.setNum(strs.elms()); FREPA(files)
      {
         Edit.FileParams &file=files[i];
         Memc<Str> fp=Split(strs[i], '?'); // file_name?params
         file.name=(fp.elms() ? fp[0] : S);
         if(fp.elms()>=2)
         {
            Memc<Str> name_vals=Split(fp[1], '&'); FREPA(name_vals)
            {
               Memc<Str> name_val=Split(name_vals[i], '=');
               if(name_val.elms()==2)file.params.New().set(name_val[0], name_val[1]);
            }
         }
      }
   }
   return files;
}
/******************************************************************************/
