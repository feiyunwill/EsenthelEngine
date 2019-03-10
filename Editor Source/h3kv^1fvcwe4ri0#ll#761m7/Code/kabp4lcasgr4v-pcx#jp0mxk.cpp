/******************************************************************************

   'Proj' is used because various world (area/object) methods operate on 'Proj'.

/******************************************************************************/
enum IMPORT_PHASE
{
   IMPORT_GET_FILES,
   IMPORT_IMPORT   ,
   IMPORT_IMPORT1  ,
   IMPORT_IMPORT2  ,
   IMPORT_IMPORT3  ,
   IMPORT_FINISHED ,
}
State                         StateImport(UpdateImport, DrawImport, InitImport, ShutImport);
Str                           ImportSrc;
BackgroundFileFind            ImportBFF;
IMPORT_PHASE                  ImportPhase=IMPORT_GET_FILES;
Memc<BackgroundFileFind.File> ImportFiles, ImportManual;
Map<Str, UID>                 ImportElms(ImportComparePath); // doesn't need to be thread-safe
Memc<Str>                     ImportMtrlImages;
Memc<Str>                     ImportObjMeshes;
Map<UID, Matrix>              ImportObjMatrix(Compare); // doesn't need to be thread-safe
/******************************************************************************/
class EE1WorldSettings : Game.WorldSettings
{
   PathSettings path;

   EE1WorldSettings& reset()
   {
      super.reset();
      path .reset();
      return T;
   }

   bool load(C Str  &name) {File f; if(f.readStdTry(name))return load(f); reset(); return false;}
   bool load(  File &f   )
   {
      switch(f.decUIntV()) // version
      {
         case 1:
         {
            if(!super.load(f))return false;
            if(!path .load(f))return false;
         }return true;

         case 0:
         {
            reset();
            f.skip(-1); if(!super.load(f))return false;
            path.reset().areaSize(areaSize());
         }return true;
      }
      reset(); return false;
   }
}
/******************************************************************************/
class EE1ObjGlobal
{
   bool load(File &f)
   {
      switch(f.decUIntV()) // version
      {
         case  1: {Rect  rect; f>>rect;} return true;
         case  0: {RectI area; f>>area;} return true;
         default: return false;
      }
   }
}
/******************************************************************************/
class EE1EditWaypoint : Game.Waypoint, EE1ObjGlobal
{
   UID id=UIDZero;
   Str name;

   bool loadData(File &f)
   {
      File temp; temp.writeMem().putUInt(CC4('W','P','N','T')); f.copy(temp);
      temp.pos(0); return Game.Waypoint.load(temp);
   }
   bool load(File &f, C Str &name)
   {
      if(id.fromHex(name))switch(f.decUIntV()) // version
      {
         case 1: if(EE1ObjGlobal.load(f))
         {
            GetStr(f, T.name);
            return loadData(f);
         }break;

         case 0:
         {
            RectI area; f>>area; GetStr(f, T.name);
         }return loadData(f);
      }
      return false;
   }
}
/******************************************************************************/
class EE1EditLake : EE1ObjGlobal
{
   UID             id=UIDZero;
   flt             depth=1;
   Memc<Memc<Vec>> polys;
   Str             material;

   bool load(File &f, C Str &name)
   {
      if(id.fromHex(name))if(super.load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>depth;
            polys.clear(); REP(f.getInt()){Memc<Vec> &poly=polys.New(); poly.setNum(f.getInt()); FREPA(poly)f>>poly[i];}
            GetStr(f, material);
         }return true;
      }
      return false;
   }
}
/******************************************************************************/
class EE1EditRiver : EE1ObjGlobal
{
   class Vtx
   {
      flt radius;
      Vec pos;
   }

   UID         id=UIDZero;
   flt         depth=1;
   Memc<Vtx  > vtxs;
   Memc<VecI2> edges;
   Str         material;

   bool load(File &f, C Str &name)
   {
      if(id.fromHex(name))if(super.load(f))switch(f.decUIntV())
      {
         case 0:
         {
            f>>depth;
            vtxs .setNum(f.getInt()); FREPA(vtxs )f>>vtxs [i];
            edges.setNum(f.getInt()); FREPA(edges)f>>edges[i];
            GetStr(f, material);
         }return true;
      }
      return false;
   }
}
/******************************************************************************/
int  ImportComparePath(C Str &a, C Str &b) {return ComparePath(a, b);}
Elm* ImportFind(C Str &name, ELM_TYPE type)
{
   if(C UID *id=ImportElms.find(SkipStartPath(name, ImportSrc)))if(Elm *elm=Proj.findElm(*id, type))return elm;
   return null;
}
UID ImportFindID(C Str &name, ELM_TYPE type)
{
   if(Elm *elm=ImportFind(name, type))return elm.id;
   return UIDZero;
}
Str ImportFindGamePath(C Str &name, ELM_TYPE type)
{
   if(Elm *elm=ImportFind(name, type))return Proj.gamePath(*elm);
   return S;
}
/******************************************************************************/
void Clean(GuiObj &obj)
{  // don't clean children because this function is already called for all elements
   switch(obj.type())
   {
      case GO_BUTTON:
      {
         if(!FExist(obj.asButton().image.name()))obj.asButton().image=null;
         if(!FExist(obj.asButton().skin .name()))obj.asButton().skin =null;
      }break;

      case GO_CHECKBOX:
      {
         if(!FExist(obj.asCheckBox().skin.name()))obj.asCheckBox().skin=null;
      }break;

      case GO_COMBOBOX:
      {
         if(!FExist(obj.asComboBox().image .name()))obj.asComboBox().image=null;
         if(!FExist(obj.asComboBox().skin().name()))obj.asComboBox().skin(null, false);
      }break;

      case GO_IMAGE:
      {
         if(!FExist(obj.asImage().image.name()))obj.asImage().image=null;
      }break;

      case GO_MENU:
      {
         if(!FExist(obj.asMenu().skin().name()))obj.asMenu().skin(null, false);
      }break;

      case GO_PROGRESS:
      {
         if(!FExist(obj.asProgress().skin.name()))obj.asProgress().skin=null;
      }break;

      case GO_REGION:
      {
         Clean(obj.asRegion().view);
         REPA(obj.asRegion().slidebar)Clean(obj.asRegion().slidebar[i]);
         if(!FExist(obj.asRegion().skin().name()))obj.asRegion().skin(null, false);
      }break;

      case GO_SLIDEBAR:
      {
         REPA(obj.asSlideBar().button)Clean(obj.asSlideBar().button[i]);
         if(!FExist(obj.asSlideBar().skin().name()))obj.asSlideBar().skin(null);
      }break;

      case GO_SLIDER:
      {
         if(!FExist(obj.asSlider().skin.name()))obj.asSlider().skin=null;
      }break;

      case GO_TAB:
      {
         if(!FExist(obj.asTab().image.name()))obj.asTab().image=null;
         if(!FExist(obj.asTab().skin .name()))obj.asTab().skin =null;
      }break;

      case GO_TABS:
      {
         REPA(obj.asTabs())Clean(obj.asTabs().tab(i));
         if(!FExist(obj.asTabs().skin().name()))obj.asTabs().skin(null);
      }break;

      case GO_TEXT:
      {
         if(!FExist(obj.asText().skin.name()))obj.asText().skin=null;
         if(!FExist(obj.asText().text_style.name()))obj.asText().text_style=null;
      }break;

      case GO_TEXTLINE:
      {
         Clean(obj.asTextLine().reset);
         if(!FExist(obj.asTextLine().skin().name()))obj.asTextLine().skin(null, false);
      }break;

      case GO_WINDOW:
      {
         REPA(obj.asWindow().button)Clean(obj.asWindow().button[i]);
         if(!FExist(obj.asWindow().skin().name()))obj.asWindow().skin(null);
      }break;
   }
}
/******************************************************************************/
void Adjust(GuiObj &obj) // here all objects should adjust paths for resources accessed using file names
{  // don't adjust children because this function is already called for all elements (calling it twice would make destroy paths because it would search in the original project for a path that was already adjusted)
   switch(obj.type())
   {
      case GO_BUTTON:
      {
         obj.asButton().image=ImportFindGamePath(obj.asButton().image.name(), ELM_IMAGE   );
         obj.asButton().skin =ImportFindGamePath(obj.asButton().skin .name(), ELM_GUI_SKIN);
      }break;

      case GO_CHECKBOX:
      {
         obj.asCheckBox().skin=ImportFindGamePath(obj.asCheckBox().skin.name(), ELM_GUI_SKIN);
      }break;

      case GO_COMBOBOX:
      {
         obj.asComboBox().image=ImportFindGamePath(obj.asComboBox().image .name(), ELM_IMAGE   );
         obj.asComboBox().skin (ImportFindGamePath(obj.asComboBox().skin().name(), ELM_GUI_SKIN), false);
      }break;

      case GO_IMAGE:
      {
         obj.asImage().image=ImportFindGamePath(obj.asImage().image.name(), ELM_IMAGE);
      }break;

      case GO_MENU:
      {
         obj.asMenu().skin(ImportFindGamePath(obj.asMenu().skin().name(), ELM_GUI_SKIN), false);
      }break;

      case GO_PROGRESS:
      {
         obj.asProgress().skin=ImportFindGamePath(obj.asProgress().skin.name(), ELM_GUI_SKIN);
      }break;

      case GO_REGION:
      {
         Adjust(obj.asRegion().view);
         REPA(obj.asRegion().slidebar)Adjust(obj.asRegion().slidebar[i]);
         obj.asRegion().skin(ImportFindGamePath(obj.asRegion().skin().name(), ELM_GUI_SKIN), false);
      }break;

      case GO_SLIDEBAR:
      {
         REPA(obj.asSlideBar().button)Adjust(obj.asSlideBar().button[i]);
         obj.asSlideBar().skin(ImportFindGamePath(obj.asSlideBar().skin().name(), ELM_GUI_SKIN));
      }break;

      case GO_SLIDER:
      {
         obj.asSlider().skin=ImportFindGamePath(obj.asSlider().skin.name(), ELM_GUI_SKIN);
      }break;

      case GO_TAB:
      {
         obj.asTab().image=ImportFindGamePath(obj.asTab().image.name(), ELM_IMAGE   );
         obj.asTab().skin =ImportFindGamePath(obj.asTab().skin .name(), ELM_GUI_SKIN);
      }break;

      case GO_TABS:
      {
         REPA(obj.asTabs())Adjust(obj.asTabs().tab(i));
         obj.asTabs().skin(ImportFindGamePath(obj.asTabs().skin().name(), ELM_GUI_SKIN));
      }break;

      case GO_TEXT:
      {
         obj.asText().skin=ImportFindGamePath(obj.asText().skin.name(), ELM_GUI_SKIN);
         obj.asText().text_style=ImportFindGamePath(obj.asText().text_style.name(), ELM_TEXT_STYLE);
      }break;

      case GO_TEXTLINE:
      {
         Adjust(obj.asTextLine().reset);
         obj.asTextLine().skin(ImportFindGamePath(obj.asTextLine().skin().name(), ELM_GUI_SKIN), false);
      }break;

      case GO_WINDOW:
      {
         REPA(obj.asWindow().button)Adjust(obj.asWindow().button[i]);
         obj.asWindow().skin(ImportFindGamePath(obj.asWindow().skin().name(), ELM_GUI_SKIN));
      }break;
   }
}
/******************************************************************************/
void Adjust(MaterialPtr &mtrl)
{
   mtrl=ImportFindGamePath(mtrl.name(), ELM_MTRL);
}
void Adjust(Mesh &mesh)
{
   REP(mesh.lods())
   {
      MeshLod &lod=mesh.lod(i); REPA(lod.parts)
      {
         MeshPart &part=lod.parts[i];
         MaterialPtr m[4]=
         {
            part.multiMaterial(0),
            part.multiMaterial(1),
            part.multiMaterial(2),
            part.multiMaterial(3),
         };
         REPA(m)Adjust(m[i]);
         part.multiMaterial(m[0], m[1], m[2], m[3]);

         REP(part.variations())if(i)
         {
            MaterialPtr m=part.variation(i); Adjust(m); part.variation(i, m);
         }
      }
   }
}
void Adjust(Heightmap &heightmap)
{
   REP(heightmap.materials())Adjust(ConstCast(heightmap.material(i)));
}
void Adjust(EditObject &obj)
{
   REPA(obj)
   {
      Param &p=obj[i];
      if(p.type==PARAM_ENUM)
         if(p.enum_type=Enums(ImportFindGamePath(Enums.name(p.enum_type), ELM_ENUM))) // adjust Enum*
      {
         int e=p.enum_type.find(p.value.s);
         if(InRange(e, *p.enum_type))p.value.id=(*p.enum_type)[e].id; // adjust enum ID
      }
   }
}
/******************************************************************************/
bool ImportFunc(Thread &thread) // 'ObjType' must be initialized because loading old format 'Object' assumes that it is set correctly
{
   ThreadMayUseGPUData();
   ObjType.del();
   Enum obj_type; // adjusted obj_type to project elm.id

   FREPA(ImportFiles)
   {
      if(thread.wantStop())return false;
      UpdateProgress.set(i, ImportFiles.elms());
      WindowSetProgress(UpdateProgress());
      if(ImportFiles[i].type==FSTD_FILE)
      {
       C Str &file=ImportFiles[i].name; Str name=GetBaseNoExt(file), ext=GetExt(file), path=GetPath(file);
         if(ext=="enum") // enum
         {
            Enum game; if(game.load(file))
            {
               if(EqualPath(file, "enum/obj_type.enum"))
               {
                  ObjType=obj_type=game;
                  FREPA(game)
                  {
                     Elm &elm=Proj.Project.newElm(game[i].name, UIDZero, ELM_OBJ_CLASS);
                     ConstCast(obj_type[i].id)=elm.id; // adjust enum ID
                     EditObject edit; edit.setType(true, elm.id, Proj.edit_path).setAccess(true, OBJ_ACCESS_CUSTOM);
                     if(elm.name=="OBJ_LIGHT_POINT")edit.setEditorType(EditObjType.elmID(EDIT_OBJ_LIGHT_POINT));else
                     if(elm.name=="OBJ_LIGHT_CONE" )edit.setEditorType(EditObjType.elmID(EDIT_OBJ_LIGHT_CONE ));else
                     if(elm.name=="OBJ_PARTICLES"  )edit.setEditorType(EditObjType.elmID(EDIT_OBJ_PARTICLES  ));
                     Save(edit, Proj.editPath(elm));
                     Object game; edit.copyTo(game, Proj, false, null, null); Save(game, Proj.gamePath(elm));
                     elm.objClassData().newData();
                     elm.objClassData().from(edit);
                  }
               }else
               {
                  Elm &elm=Proj.Project.newElm(game.name, Proj.getPathID(path), ELM_ENUM);
                  ElmEnum *enum_data=elm.enumData();
                  enum_data.newData();
                  enum_data.src_file=MakeFullPath(file, FILE_DATA);
                  FCopy(file, Proj.gamePath(elm));
                  EditEnums edit; edit.create(game); Save(edit, Proj.editPath(elm));
                 *ImportElms(file)=elm.id;
               }
            }
         }else
         if(ext=="anim") // animation
         {
            Animation game; if(game.load(file))
            {
               Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_ANIM);
               elm.animData().newData();
               elm.animData().loop(game.loop()).linear(game.linear());
               FCopy(file, Proj.gamePath(elm));
              *ImportElms(file)=elm.id;
            }
         }else
         if(ext=="font") // font
         {
            Font game; if(game.load(file))
            {
               Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_FONT);
               elm.fontData().newData();
               FCopy(file, Proj.gamePath(elm));
               EditFont edit; edit.german=game.hasChar('ä'); edit.french=game.hasChar('à'); edit.polish=game.hasChar('ą'); edit.russian=game.hasChar('л'); edit.chinese=game.hasChar('嗨');
               edit.german_time.getUTC(); edit.french_time.getUTC(); edit.polish_time.getUTC(); edit.russian_time.getUTC(); edit.chinese_time.getUTC();
               edit.size=game.height(); edit.font=name;
               edit.size_time.getUTC(); edit.font_time.getUTC();
               Save(edit, Proj.editPath(elm));
              *ImportElms(file)=elm.id;
            }
         }else
         if(ext=="ogg" || ext=="wav") // sound
         {
            Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_SOUND);
            elm.soundData().newData();
            FCopy(file, Proj.gamePath(elm));
           *ImportElms(file)=elm.id;
         }else
         if(ext=="ogm" || ext=="ogv" || ext=="theora" || ext=="webm") // video
         {
            Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_VIDEO);
            elm.videoData().newData();
            FCopy(file, Proj.gamePath(elm));
           *ImportElms(file)=elm.id;
         }else
         if(ext=="mtrl") // material
         {
            Material game; if(game.load(file))
            {
               Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_MTRL);
               EditMaterial edit; edit.create(game); // create from material

               // set textures
               Image base_1, detail_map; if(game.base_1)UpdateMtrlTex(*game.base_1, base_1); if(game.detail_map)UpdateMtrlTex(*game.detail_map, detail_map);
               if(game.        base_0)ImageProps(*game.        base_0, &edit.    base_0_tex, null,  ForceHQMtrlBase0  ? FORCE_HQ : 0              );else edit.    base_0_tex.zero();
               if(game.        base_1)ImageProps(              base_1, &edit.    base_1_tex, null,  ForceHQMtrlBase1  ? FORCE_HQ : 0              );else edit.    base_1_tex.zero();
               if(game.    detail_map)ImageProps(          detail_map, &edit.    detail_tex, null, (ForceHQMtrlDetail ? FORCE_HQ : 0)|IGNORE_ALPHA);else edit.    detail_tex.zero();
               if(game.     macro_map)ImageProps(*game.     macro_map, &edit.     macro_tex, null,                                    IGNORE_ALPHA);else edit.     macro_tex.zero();
               if(game.reflection_map)ImageProps(*game.reflection_map, &edit.reflection_tex, null,                                    IGNORE_ALPHA);else edit.reflection_tex.zero();
               if(game.     light_map)ImageProps(*game.     light_map, &edit.     light_tex, null,                                    IGNORE_ALPHA);else edit.     light_tex.zero();
               if(edit.    base_0_tex.valid())if(Proj.includeTex(edit.    base_0_tex))game.        base_0->save(Proj.texPath(edit.    base_0_tex));
               if(edit.    base_1_tex.valid())if(Proj.includeTex(edit.    base_1_tex))             base_1. save(Proj.texPath(edit.    base_1_tex));
               if(edit.    detail_tex.valid())if(Proj.includeTex(edit.    detail_tex))         detail_map. save(Proj.texPath(edit.    detail_tex));
               if(edit.     macro_tex.valid())if(Proj.includeTex(edit.     macro_tex))game.     macro_map->save(Proj.texPath(edit.     macro_tex));
               if(edit.reflection_tex.valid())if(Proj.includeTex(edit.reflection_tex))game.reflection_map->save(Proj.texPath(edit.reflection_tex));
               if(edit.     light_tex.valid())if(Proj.includeTex(edit.     light_tex))game.     light_map->save(Proj.texPath(edit.     light_tex));

               Str b0=MakeFullPath(game.base_0.name(), FILE_DATA), b1=MakeFullPath(game.base_1.name(), FILE_DATA), d=MakeFullPath(game.detail_map.name(), FILE_DATA), m=MakeFullPath(game.macro_map.name(), FILE_DATA), r=MakeFullPath(game.reflection_map.name(), FILE_DATA), l=MakeFullPath(game.light_map.name(), FILE_DATA);
               ImportMtrlImages.binaryInclude(SkipStartPath(b0, ImportSrc), ImportComparePath); b0.tailSlash(true);
               ImportMtrlImages.binaryInclude(SkipStartPath(b1, ImportSrc), ImportComparePath); b1.tailSlash(true);
               ImportMtrlImages.binaryInclude(SkipStartPath(d , ImportSrc), ImportComparePath); d .tailSlash(true);
               ImportMtrlImages.binaryInclude(SkipStartPath(m , ImportSrc), ImportComparePath);
               ImportMtrlImages.binaryInclude(SkipStartPath(r , ImportSrc), ImportComparePath);
               ImportMtrlImages.binaryInclude(SkipStartPath(l , ImportSrc), ImportComparePath);

               edit.   color_map=(b0.is() ? b0+"xyz" : S); edit.     color_map_time.getUTC();
               edit.   alpha_map.clear();                  edit.     alpha_map_time.getUTC();
               edit.    bump_map.clear();                  edit.      bump_map_time.getUTC();
               edit.    glow_map.clear();                  edit.      glow_map_time.getUTC();
               edit.  normal_map.clear();                  edit.    normal_map_time.getUTC();
               edit.specular_map.clear();                  edit.  specular_map_time.getUTC();
               edit.detail_color =(d.is() ? d+"x"  : S);   edit.    detail_map_time.getUTC();
               edit.detail_bump  =(d.is() ? d+"z"  : S);
               edit.detail_normal=(d.is() ? d+"wy" : S);
               edit.     macro_map=m;                      edit.     macro_map_time.getUTC();
               edit.reflection_map=r;                      edit.reflection_map_time.getUTC();
               edit.     light_map=l;                      edit.     light_map_time.getUTC();
               if(b1.is())
               {
                  edit.    bump_map=(b0.is() ? b0+"w" : S);
                  edit.  normal_map=b1+"wy";
                  edit.   alpha_map=b1+"z";
                  edit.    glow_map=b1+"z";
                  edit.specular_map=b1+"x";
               }else
               if(b0.is())
               {
                  edit.alpha_map=b0+"w";
               }

               // save
               Save(edit, Proj.editPath(elm));
               edit.copyTo(game, Proj); Save(game, Proj.gamePath(elm));

               // finalize
               ElmMaterial *mtrl_data=elm.mtrlData();
               mtrl_data.newData();
               mtrl_data.src_file=MakeFullPath(file, FILE_DATA);
               mtrl_data.from(edit);
              *ImportElms(file)=elm.id;
            }
         }else
         if(ext=="obj") // object (only create elements but don't set data)
         {
            Object game; if(game.load(file))
            {
               Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_OBJ);
               elm.objData().newData();
              *ImportElms(file)=elm.id;
            }
         }
      }
   }

   // 2nd phase (images after materials, text styles after fonts, objects after creating them before)
   ImportPhase=IMPORT_IMPORT1;
   FREPA(ImportFiles)
   {
      if(thread.wantStop())return false;
      UpdateProgress.set(i, ImportFiles.elms());
      WindowSetProgress(UpdateProgress());
      if(ImportFiles[i].type==FSTD_FILE)
      {
         C Str &file=ImportFiles[i].name; Str name=GetBaseNoExt(file), ext=GetExt(file), path=GetPath(file);
         if(ext=="img") // image
         {
            Image game;
            if(!ImportMtrlImages.binaryHas(file, ImportComparePath)) // if not material texture
               if(game.load(file))
                  if(game.is())
            {
               Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_IMAGE);
               ElmImage *image_data=elm.imageData();
               image_data.newData();
               image_data.src_file=MakeFullPath(file, FILE_DATA);
               if(ImageTI[game.type()].compressed)elm.imageData().type=ElmImage.COMPRESSED;else 
               if(        game.type()==IMAGE_A8  )elm.imageData().type=ElmImage.ALPHA;else 
                                                  elm.imageData().type=ElmImage.FULL;
               elm.imageData().mode=game.mode();
               elm.imageData().mipMaps(game.mipMaps()>1);
               elm.imageData().pow2(IsPow2(game.w()) && IsPow2(game.h()));
               elm.imageData().hasAlpha(HasAlpha(game));
               elm.imageData().hasColor(HasColor(game));
               game.ExportWEBP(Proj.editPath(elm), 1, 1);
               FCopy(file, Proj.gamePath(elm));
              *ImportElms(file)=elm.id;
            }
         }else
         if(ext=="txds") // text styles
         {
            TextStyle game; if(game.load(file))
            {
               Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_TEXT_STYLE);
               EditTextStyle edit; edit.create(game, ImportFindID(game.font().name(), ELM_FONT)); Save(edit, Proj.editPath(elm));
               edit.copyTo(game, Proj); Save(game, Proj.gamePath(elm));
               elm.textStyleData().newData();
               elm.textStyleData().from(edit);
              *ImportElms(file)=elm.id;
            }
         }else
         if(ext=="obj") // object
         {
            if(Elm *elm=ImportFind(file, ELM_OBJ))
            {
               Object        game; game.load(file);
               UID           type=obj_type.elmID(ObjType.find(game.type())); // adjust ID in case source uses ID not listed in ObjType
               EditObjectPtr base; if(Elm *base_elm=ImportFind(game.base().name(), ELM_OBJ))base=Proj.editPath(*base_elm);else if(type.valid())base=Proj.editPath(type);
               game.type(false); type.zero(); // clear type as it is always taken from base (OBJ_CLASS)
               if(game.access()==OBJ_ACCESS_CUSTOM)game.access(false); // use access from base
               else                                base=null;          // we're not using custom class, so disable base
               EditObject edit; edit.create(game, type, base); Adjust(edit); Save(edit, Proj.editPath(*elm));
               UID *mesh_id=null, *phys_id=null;
               Matrix matrix=game.matrixFinal(); Pose transform=matrix;

               ImportObjMeshes.binaryInclude(Str(game.mesh().name()), ImportComparePath);
              *ImportObjMatrix(elm.id)=~matrix;

               if(game.mesh() && game.mesh()->is()
               || game.phys() && game.phys()->is())
               {
                  Elm &mesh_elm=Proj.Project.newElm(ElmNameMesh, elm.id, ELM_MESH); mesh_id=&mesh_elm.id;
                  ElmMesh *mesh_data=mesh_elm.meshData();
                  mesh_data.newData();
                  mesh_data.src_file=MakeFullPath(game.mesh().name(), FILE_DATA);
                  mesh_data.transform=transform;
                  mesh_data.obj_id=elm.id;

                  Mesh edit_mesh, game_mesh; if(game.mesh())edit_mesh.create(*game.mesh());
                  edit_mesh.setBase().delRender();

                  // adjust materials
                  Adjust(edit_mesh);

                  edit_mesh.drawGroupEnum(null);

                  // skeleton
                  if(edit_mesh.skeleton())
                  {
                     Skeleton skel=*edit_mesh.skeleton(); EditSkeleton edit_skel; edit_skel.create(skel, null); skel.transform(matrix);
                     Elm &skel_elm=Proj.Project.newElm(ElmNameSkel, mesh_elm.id, ELM_SKEL); mesh_elm.meshData().skel_id=skel_elm.id;
                     ElmSkel *skel_data=skel_elm.skelData();
                     skel_data.newData();
                     skel_data.src_file=MakeFullPath(Skeletons.name(edit_mesh.skeleton()), FILE_DATA);
                     skel_data.mesh_id  =mesh_elm.id;
                     skel_data.transform=transform;
                     Save(edit_skel, Proj.editPath(skel_elm));
                     Save(     skel, Proj.gamePath(skel_elm));
                     edit_mesh.skeleton(null);
                  }
                  Skeleton *body_skel; Proj.getMeshSkels(mesh_elm.meshData(), null, &body_skel);
                  EditToGameMesh(edit_mesh, game_mesh, body_skel, null, &matrix);
                  Save(edit_mesh, Proj.editPath(mesh_elm), Proj.game_path); // must use 'game_path'
                  Save(game_mesh, Proj.gamePath(mesh_elm));
                  mesh_elm.meshData().from(game_mesh);

                  // phys body
                  if(game.phys() && game.phys()->is())
                  {
                     PhysBody phys=*game.phys(); phys.transform(matrix);
                     Elm &phys_elm=Proj.Project.newElm(ElmNamePhys, mesh_elm.id, ELM_PHYS); phys_id=&phys_elm.id; mesh_elm.meshData().phys_id=phys_elm.id;
                     Save(phys, Proj.gamePath(phys_elm));
                     ElmPhys *phys_data=phys_elm.physData();
                     phys_data.newData();
                     phys_data.src_file=MakeFullPath(game.phys().name(), FILE_DATA);
                     phys_data.density  =phys.density;
                     phys_data.mesh_id  =mesh_elm.id;
                     phys_data.transform=transform;
                     phys_data.from(phys);
                  }
               }
               edit.copyTo(game, Proj, true, mesh_id, phys_id); Save(game, Proj.gamePath(*elm));
               elm.objData().from(edit); if(mesh_id)elm.objData().mesh_id=*mesh_id;
            }
         }
      }
   }

   // 3rd phase (Panels after importing images, non-object meshes after importing all objects, particles after importing images)
   ImportPhase=IMPORT_IMPORT2;
   FREPA(ImportFiles)
   {
      if(thread.wantStop())return false;
      UpdateProgress.set(i, ImportFiles.elms());
      WindowSetProgress(UpdateProgress());
      if(ImportFiles[i].type==FSTD_FILE)
      {
         C Str &file=ImportFiles[i].name; Str name=GetBaseNoExt(file), ext=GetExt(file), path=GetPath(file);
         if(ext=="gstl") // Panel (old GuiStyle)
         {
            Panel game; if(game.load(file))
            {
               Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_PANEL);
               EditPanel edit;
               edit.create(game, ImportFindID(game.       center_image.name(), ELM_IMAGE),
                                 ImportFindID(game.       border_image.name(), ELM_IMAGE),
                                 ImportFindID(game.          top_image.name(), ELM_IMAGE),
                                 ImportFindID(game.       bottom_image.name(), ELM_IMAGE),
                                 ImportFindID(game.   left_right_image.name(), ELM_IMAGE),
                                 ImportFindID(game.   top_corner_image.name(), ELM_IMAGE),
                                 ImportFindID(game.bottom_corner_image.name(), ELM_IMAGE),
                                 ImportFindID(game.        panel_image.name(), ELM_PANEL_IMAGE)); Save(edit, Proj.editPath(elm));
               edit.copyTo(game, Proj); Save(game, Proj.gamePath(elm));
               elm.panelData().newData();
               elm.panelData().from(edit);
              *ImportElms(file)=elm.id;
            }
         }else
         if(ext=="mesh") // mesh
         {
            Mesh edit_mesh;
            if(!ImportObjMeshes.binaryHas(file, ImportComparePath)) // if not object mesh
               if(edit_mesh.load(file))
                  if(edit_mesh.is())
            {
               Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_OBJ);
               elm.objData().newData();
              *ImportElms(file)=elm.id; // src mesh is now ELM_OBJ

               EditObject edit; Save(edit, Proj.editPath(elm));
               UID *mesh_id=null, *phys_id=null;

               Elm &mesh_elm=Proj.Project.newElm(ElmNameMesh, elm.id, ELM_MESH); mesh_id=&mesh_elm.id;
               ElmMesh *mesh_data=mesh_elm.meshData();
               mesh_data.newData();
               mesh_data.src_file=file;
               mesh_data.obj_id=elm.id;

               edit_mesh.setBase().delRender();

               // adjust materials
               Adjust(edit_mesh);

               edit_mesh.drawGroupEnum(null);

               // skeleton
               if(edit_mesh.skeleton())
               {
                  Elm &skel_elm=Proj.Project.newElm(ElmNameSkel, elm.id, ELM_SKEL); mesh_elm.meshData().skel_id=skel_elm.id;
                  ElmSkel *skel_data=skel_elm.skelData();
                  skel_data.newData();
                  skel_data.src_file=MakeFullPath(Skeletons.name(edit_mesh.skeleton()), FILE_DATA);
                  skel_data.mesh_id=mesh_elm.id;
                  EditSkeleton edit_skel; edit_skel.create(*edit_mesh.skeleton(), null);
                  Save( edit_skel           , Proj.editPath(skel_elm));
                  Save(*edit_mesh.skeleton(), Proj.gamePath(skel_elm));
                  edit_mesh.skeleton(null);
               }
               Skeleton *body_skel; Proj.getMeshSkels(mesh_elm.meshData(), null, &body_skel);
               Mesh game_mesh; EditToGameMesh(edit_mesh, game_mesh, body_skel, null, null);
               Save(edit_mesh, Proj.editPath(mesh_elm), Proj.game_path); // must use 'game_path'
               Save(game_mesh, Proj.gamePath(mesh_elm));
               mesh_elm.meshData().from(game_mesh);

               // phys body
               /*if(phys)
               {
                  Elm &phys_elm=Proj.Project.newElm(name, elm.id, ELM_PHYS); phys_id=&phys_elm.id; mesh_elm.meshData().phys_id=phys_elm.id;
                  Save(phys, Proj.gamePath(phys_elm));
                  phys_elm.physData().newData();
                  phys_elm.physData().density=phys.density;
                  phys_elm.physData().mesh_id=mesh_elm.id;
                  phys_elm.physData().from(phys);
                  phys_elm.physData().src_file=MakeFullDataPath(phys.name());
               }*/
               Object game; edit.copyTo(game, Proj, true, mesh_id, phys_id); Save(game, Proj.gamePath(elm));
               elm.objData().from(edit); if(mesh_id)elm.objData().mesh_id=*mesh_id;
            }
         }else
         if(ext=="particle") // particles
         {
            Particles p;
            if(p.load(file))
            {
               Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_OBJ);
               EditObject edit;
                   Object game;

               if(Elm *elm=ImportFind(p.        image.name(), ELM_IMAGE))edit.New().setName(        "Image").setAsIDArray(elm.id).id.set( 5, 0, 0, 0);
               if(Elm *elm=ImportFind(p.palette_image.name(), ELM_IMAGE))edit.New().setName("Palette Image").setAsIDArray(elm.id).id.set(34, 0, 0, 0);
               if(       p.image_x_frames> 1 )edit.New().setName("Image Frames X"    ).forceInt  (p.image_x_frames).id.set( 6, 0, 0, 0);
               if(       p.image_y_frames> 1 )edit.New().setName("Image Frames Y"    ).forceInt  (p.image_y_frames).id.set( 7, 0, 0, 0);
               if(!Equal(p.image_speed   , 1))edit.New().setName("Image Speed"       ).forceFlt  (p.image_speed   ).id.set( 8, 0, 0, 0);
               if(!      p.reborn            )edit.New().setName("Reborn"            ).forceBool (p.reborn        ).id.set( 9, 0, 0, 0);
                                              edit.New().setName("Palette Mode"      ).forceBool (p.palette()     ).id.set(10, 0, 0, 0);
               if(       p.paletteIndex()    )edit.New().setName("Palette Index"     ).forceBool (p.paletteIndex()!=0).id.set(11, 0, 0, 0);
               if(       p.smooth_fade       )edit.New().setName("Smooth Fade"       ).forceBool (p.smooth_fade   ).id.set(12, 0, 0, 0);
                                              edit.New().setName("Elements"          ).forceInt  (p.p.elms()      ).id.set(13, 0, 0, 0);
                                              edit.New().setName("Color"             ).forceColor(p.color         ).id.set( 1, 0, 0, 0);
               if(       p.glow              )edit.New().setName("Glow"              ).forceInt  (p.glow          ).id.set(14, 0, 0, 0);
                                              edit.New().setName("Radius"            ).forceFlt  (p.radius        ).id.set(15, 0, 0, 0);
               if(!Equal(p.radius_random , 0))edit.New().setName("Radius Random"     ).forceFlt  (p.radius_random ).id.set(32, 0, 0, 0);
               if(!Equal(p.radius_growth , 1))edit.New().setName("Radius Growth"     ).forceFlt  (p.radius_growth ).id.set(16, 0, 0, 0);
               if(!Equal(p.offset_range  , 0))edit.New().setName("Offset Range"      ).forceFlt  (p.offset_range  ).id.set(17, 0, 0, 0);
               if(!Equal(p.offset_speed  , 1))edit.New().setName("Offset Speed"      ).forceFlt  (p.offset_speed  ).id.set(18, 0, 0, 0);
                                              edit.New().setName("Life"              ).forceFlt  (p.life          ).id.set(19, 0, 0, 0);
               if(!Equal(p.life_random   , 0))edit.New().setName("Life Random"       ).forceFlt  (p.life_random   ).id.set(33, 0, 0, 0);
               if(!Equal(p.glue          , 0))edit.New().setName("Glue"              ).forceFlt  (p.glue          ).id.set(20, 0, 0, 0);
               if(!Equal(p.damping       , 0))edit.New().setName("Velocity Damping"  ).forceFlt  (p.damping       ).id.set(21, 0, 0, 0);
               if(!Equal(p.ang_vel       , 0))edit.New().setName("Angular Velocity"  ).forceFlt  (p.ang_vel       ).id.set(22, 0, 0, 0);
                                              edit.New().setName("Random Velocity"   ).forceFlt  (p.vel_random    ).id.set(23, 0, 0, 0);
               if(!Equal(p.vel_constant.x, 0))edit.New().setName("Initial Velocity X").forceFlt  (p.vel_constant.x).id.set(24, 0, 0, 0);
               if(!Equal(p.vel_constant.y, 0))edit.New().setName("Initial Velocity Y").forceFlt  (p.vel_constant.y).id.set(25, 0, 0, 0);
               if(!Equal(p.vel_constant.z, 0))edit.New().setName("Initial Velocity Z").forceFlt  (p.vel_constant.z).id.set(26, 0, 0, 0);
               if(!Equal(p.accel.x       , 0))edit.New().setName("Acceleration X"    ).forceFlt  (p.accel.x       ).id.set(27, 0, 0, 0);
               if(!Equal(p.accel.y       , 0))edit.New().setName("Acceleration Y"    ).forceFlt  (p.accel.y       ).id.set(28, 0, 0, 0);
               if(!Equal(p.accel.z       , 0))edit.New().setName("Acceleration Z"    ).forceFlt  (p.accel.z       ).id.set(29, 0, 0, 0);
               if( p.sourceType()==PARTICLE_STATIC_SHAPE)edit.New().setName("Source Shape").forceInt(p.shape.type ).id.set(30, 0, 0, 0);
               if(!p.inside_shape            )edit.New().setName("Inside Source"     ).forceBool (p.inside_shape  ).id.set(31, 0, 0, 0);

               Save(edit, Proj.editPath(elm)); edit.copyTo(game, Proj, true, null, null);
               Save(game, Proj.gamePath(elm));
               elm.objData().newData(); elm.objData().from(edit);
              *ImportElms(file)=elm.id;
            }
         }
      }
   }

   // 4th phase (gui objects after importing images text styles and Panels, worlds after importing all objects and meshes)
   ImportPhase=IMPORT_IMPORT3;
   FREPA(ImportFiles)
   {
      if(thread.wantStop())return false;
      UpdateProgress.set(i, ImportFiles.elms());
      WindowSetProgress(UpdateProgress());
      C Str &file=ImportFiles[i].name; Str name=GetBaseNoExt(file), ext=GetExt(file), path=GetPath(file);
      if(ImportFiles[i].type==FSTD_FILE)
      {
         if(ext=="gobj") // gui objects
         {
            GuiObjs game; if(game.load(file))
            {
               REP(GO_NUM)if(C MembConst<const_mem_addr GuiObj> *objs=game.objects(GUI_OBJ_TYPE(i)))REPA(*objs)Adjust((*objs)[i]);
               Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_GUI);
               Save(game, Proj.gamePath(elm));
               elm.guiData().newData();
              *ImportElms(file)=elm.id;
            }
         }
      }else
      if(ImportFiles[i].type==FSTD_DIR)
      {
         if(ext=="world") // world
         {
            EE1WorldSettings settings;
            Str edit=file; edit.tailSlash(true)+="Edit/";
            if(settings.load(edit+"Settings"))
            {
               Elm &elm=Proj.Project.newElm(name, Proj.getPathID(path), ELM_WORLD);
              *ImportElms(file)=elm.id;
               elm.worldData().newData();
               elm.worldData().area_size=            settings.areaSize();
               elm.worldData().hm_res   =NearestPow2(settings.hmRes   ());
               elm.worldData().ctrlRadius(settings.path.ctrlRadius());
               elm.worldData().ctrlHeight(settings.path.ctrlHeight());
               elm.worldData().maxClimb  (settings.path.maxClimb  ());
               elm.worldData().maxSlope  (settings.path.maxSlope  ());
               elm.worldData().cellSize  (settings.path.cellSize  ());
               elm.worldData().cellHeight(settings.path.cellHeight());
               Proj.makeGameVer(elm); // create paths and save world settings (call this after setting them)

               WorldVer *world_ver=Proj.worldVerRequire(elm.id); world_ver.changed=true;
               for(FileFind ff(MakeFullPath(edit+"Area", FILE_DATA)); ff(); )if(ff.type==FSTD_FILE && !GetExt(ff.name).is())
               {
                  if(thread.wantStop())return false;
                  VecI2 xy=TextVecI2(ff.name);
                  File f; if(f.readStdTry(ff.pathName()))
                  {
                     Heightmap h;
                     Memt<Game.Area.Data.AreaObj> objs; // !! Warning: this should actually be Edit.Obj !! however we'd need 'Object.loadData' method which is hidden, and 'AreaObj' provides same save/load functionality as 'Edit.Obj'
                     for(ChunkReader cr(f); File *f=cr(); )
                     {
                        if(cr.name()=="Heightmap")switch(cr.ver())
                        {
                           case 0: h.load(*f); break;
                        }else
                        if(cr.name()=="Object")switch(cr.ver())
                        {
                           case 1: REP(f.getInt())if(!objs.New().load(*f)){objs.removeLast(); break;} break;
                        }
                     }
                     if(h.is() || objs.elms())
                     {
                        Area      area(xy, null); area.ver=world_ver.areas(xy); // create AreaVer (needed for rebuild)
                        Memt<Obj> area_objs;
                        if(h.is())
                        {
                           Adjust(h);
                           Swap(SCAST(Heightmap, *New(Delete(area.hm))), h);
                           if(area.ver)
                           {
                              area.ver.hm_height_time.getUTC();
                              area.ver.hm_mtrl_time  .getUTC();
                              area.ver.hm_color_time .getUTC();
                           }
                           world_ver.rebuildAreaNeighbor(xy, AREA_REBUILD_HM|AREA_REBUILD_HM_MESH|AREA_REBUILD_HM_MESH_SIMPLIFY|AREA_REBUILD_HM_PHYS); // rebuild after settings AreaVer
                        }
                        REPA(objs)
                        {
                           Game.Area.Data.AreaObj &src =objs[i];
                           UID                     type=obj_type.elmID(ObjType.find(src.type())); // adjust ID in case source uses ID not listed in ObjType
                           EditObjectPtr           base;
                           if(src.access()!=OBJ_ACCESS_CUSTOM)type.zero(); // clear class if we're not using custom
                           Elm *base_elm=ImportFind(src.base().name(), ELM_OBJ); if(!base_elm)base_elm=ImportFind(src.mesh().name(), ELM_OBJ); // if object base was not found, then try checking its mesh (because earlier we've imported non-object meshes as objects)
                           if(  base_elm) // create only if we've found ELM_OBJ base
                           {
                              base=Proj.editPath(*base_elm);
                              Obj &obj=area_objs.New();
                              obj.ObjData.create(src, type, base);
                              Adjust(obj.params);
                              if(C Matrix *matrix=ImportObjMatrix.find(base_elm.id))obj.matrix=(*matrix)*obj.matrix; // object meshes are already transformed
                              world_ver.changedObj(obj, xy);
                           }
                        }
                        if(area_objs.elms())
                        {
                           FREPA(area_objs)area.objs.add(&area_objs[i]);
                           if(area.ver)area.ver.obj_ver.randomize();
                           world_ver.rebuildAreaNeighbor(xy, AREA_REBUILD_GAME_AREA_OBJS); // rebuild after settings AreaVer
                        }
                        world_ver.rebuildAreaNeighbor(xy, AREA_REBUILD_EMBED_OBJ|AREA_REBUILD_WATER|AREA_REBUILD_PATH); // rebuild after settings AreaVer
                        area.saveEdit(Proj.editAreaPath(elm.id, xy));
                        area.objs.del(); // delete obj pointer container manually before objects are deleted (otherwise Area destructor would be accessing removed objects)
                     }
                  }
               }
               Pak global;
               if(global.load(MakeFullPath(edit+"Global/Waypoints.pak", FILE_DATA)))FREPA(global)if(global.file(i).data_size) // waypoints
               {
                  EE1EditWaypoint src; File f; if(f.readTry(global.file(i), global))if(src.load(f, global.file(i).name))
                  {
                      EditWaypoint edit; edit.create(src, src.name); Save(edit, Proj.editWaypointPath(elm.id, src.id));
                     Game.Waypoint game; edit.copyTo(game         ); Save(game, Proj.gameWaypointPath(elm.id, src.id));
                     world_ver.changedWaypoint(src.id);
                  }
               }
               if(global.load(MakeFullPath(edit+"Global/Lakes.pak", FILE_DATA)))FREPA(global)if(global.file(i).data_size) // lakes
               {
                  EE1EditLake src; File f; if(f.readTry(global.file(i), global))if(src.load(f, global.file(i).name))
                  {
                     Lake lake; lake.setDepth(src.depth); Swap(lake.polys, src.polys); lake.polys_time.getUTC();
                     Save(lake, Proj.editLakePath(elm.id, src.id));
                     world_ver.changedLake(src.id);
                  }
               }
               if(global.load(MakeFullPath(edit+"Global/Rivers.pak", FILE_DATA)))FREPA(global)if(global.file(i).data_size) // rivers
               {
                  EE1EditRiver src; File f; if(f.readTry(global.file(i), global))if(src.load(f, global.file(i).name))
                  {
                     River river; river.setDepth(src.depth); FREPA(src.vtxs)river.vtxs.New().set(src.vtxs[i].radius, src.vtxs[i].pos); Swap(river.edges, src.edges); river.vtx_edge_time.getUTC();
                     Save(river, Proj.editRiverPath(elm.id, src.id));
                     world_ver.changedRiver(src.id);
                  }
               }
            }
         }
      }
   }

   // move materials used by only one object to inside of the obj
   REPA(Proj.elms)
   {
      Elm &elm=Proj.elms[i]; if(ElmMesh *mesh_data=elm.meshData())
      {
         REPA(mesh_data.mtrl_ids)
         {
          C UID &mtrl_id=mesh_data.mtrl_ids[i];
            REPA(Proj.elms) // check other meshes
            {
               Elm &elm_other=Proj.elms[i]; if(ElmMesh *mesh_data=elm_other.meshData())if(&elm_other!=&elm)if(mesh_data.mtrl_ids.binaryHas(mtrl_id, Compare))goto used;
            }
            if(Elm *mtrl_elm=Proj.findElm(mtrl_id, ELM_MTRL))if(Elm *obj=Proj.meshToObjElm(&elm))mtrl_elm.setParent(obj); // set to object
         used:;
         }
      }
   }

   ImportPhase=IMPORT_FINISHED;
   return false;
}
bool ImportFilter(C Str &name)
{
   Str ext=GetExt(name);
   if( ext=="world" || ext=="atlas") // world and atlas
      if(FileInfoSystem(name).type==FSTD_DIR) // folder
   {
      BackgroundFileFind.File &file=ImportManual.New(); file.get(name); file.name=name; // add this element manually
      return false; // skip files inside those folders
   }
   return true;
}
/******************************************************************************/
bool InitImport()
{
   ImportSrc.tailSlash(false);
   if(FileInfoSystem(ImportSrc).type!=FSTD_DIR)StateProjectList.set(StateFadeTime);else
   {
      UID id; id.randomizeValid(); Str name=GetBase(ImportSrc); if(name=="Data")name=GetBase(GetPath(ImportSrc)); Str path=ProjectsPath+EncodeFileName(id);
      Str error; LOAD_RESULT result=Proj.Project.open(id, name, path, error); // open using 'Project' method
      if(!FExistSystem(path) || !LoadOK(result)){StateProjectList.set(StateFadeTime); Gui.msgBox(S, S+"Can't write to:\"\n"+ProjectsPath+'"');}else
      {
         DataPath(ImportSrc);
         ImportPhase=IMPORT_GET_FILES;
         ImportBFF.find(ImportSrc, ImportFilter);
         UpdateProgress.create(Rect_C(0, -0.1, 1, 0.045));
      }
   }
   return true; 
}
void ShutImport()
{
   UpdateThread  .del(); // delete the thread first
   UpdateProgress.del();
   if(ImportPhase!=IMPORT_FINISHED)
   {
      FDelDirs(Proj.path); // delete what was imported until now
      Proj.Project.del();
   }
   ImportBFF       .del();
   ImportFiles     .del(); ImportManual.del(); // delete after BFF
   ImportElms      .del();
   ImportMtrlImages.del();
   ImportObjMeshes .del();
   ImportObjMatrix .del();
   DataPath(S);
   WindowSetNormal();
}
/******************************************************************************/
bool UpdateImport()
{
   if(Kb.bp(KB_ESC)){StateProjectList.set(StateFadeTime); Gui.msgBox(S, "Importing breaked on user request");} // cancel importing
   if(ImportPhase==IMPORT_GET_FILES)
   {
      if(ImportBFF.finished())
      {
         ImportBFF.getFiles(ImportFiles);
         FREPA(ImportManual)Swap(ImportFiles.New(), ImportManual[i]);
         REPAO(ImportFiles).name=SkipStartPath(ImportFiles[i].name, ImportSrc);
         ImportFiles.sort(BackgroundFileFind.Compare);
         ImportPhase=IMPORT_IMPORT;
         UpdateThread.create(ImportFunc); // create thread after all other stuff
      }
   }
   if(ImportPhase==IMPORT_FINISHED)
   {
      WindowFlash();
      Proj.save();
      Str error; Proj.open(Proj.id, Proj.name, Proj.path, error); // reopen using 'ProjectEx' method
      SetProjectState();
   }
   Time.wait(1000/30); // limit to 30 fps
   Server.update(null, true);
   if(Ms.bp(3))WindowToggle();
   return true;
}
/******************************************************************************/
void DrawImport()
{
   D.clear(BackgroundColor());
   D.text(Rect(Vec2(0, 0.2)), S+"Importing Project\n\""+ImportSrc+'"');
   switch(ImportPhase)
   {
      case IMPORT_GET_FILES:
      {
         D.text(0, 0, "Getting list of files");
      }break;

      case IMPORT_IMPORT :
      case IMPORT_IMPORT1:
      case IMPORT_IMPORT2:
      case IMPORT_IMPORT3:
      {
         D.text(0, 0, S+"Importing files "+(ImportPhase-IMPORT_IMPORT+1)+"/4");

         GuiPC gpc;
         gpc.visible=gpc.enabled=true; 
         gpc.client_rect=gpc.clip.set(-D.w(), -D.h(), D.w(), D.h());
         gpc.offset.zero();
         UpdateProgress.draw(gpc);
         D.clip();
      }break;
   }
}
/******************************************************************************/
