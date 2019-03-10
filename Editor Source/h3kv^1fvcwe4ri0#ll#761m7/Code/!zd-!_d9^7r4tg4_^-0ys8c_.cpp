/******************************************************************************/
class ProjectSettings : ClosableWindow
{
   class CompressType
   {
      COMPRESS_TYPE type;
      cchar        *desc;
      cchar8       *name;
   }

   Button    randomize_key;
   TextLine  cipher_key;
   Text      compress, mtrl_simplify_t, text_data_t;
   ComboBox  cipher_type, compress_type, compress_level, mtrl_simplify, text_data;
   Button    ok;
   TextBlack ts;

   static CompressType cmpr_type[]=
   {
      {COMPRESS_NONE , u"Biggest Pak size (100%) and best performance (26x) for accessing files"},
      {COMPRESS_LZ4  , u"Medium Pak size (~70%) and good performance (16x) for accessing files"},
    //{COMPRESS_ZLIB , u"Small Pak size (~64%) and medium performance (5x) for accessing files"}, now removed since ZSTD is better
      {COMPRESS_ZSTD , u"Small Pak size (~63%) and medium performance (8x) for accessing files"},
      {COMPRESS_LZHAM, u"Small Pak size (~57%) and slow performance (2x) for accessing files"},
      {COMPRESS_LZMA , u"Smallest Pak size (~55%) and slowest performance (1x) for accessing files"},
   };
   static cchar8 *cmpr_lvl[]=
   {
      "0",
      "1",
      "2",
      "3",
      "4",
      "5",
      "6",
      "7",
      "8",
      "9",
      "10",
      "11",
      "12",
      "13",
      "14",
      "15",
      "16",
      "17",
      "18",
      "19",
      "20",
      "21",
      "22",
   };

   static NameDesc mtrl_simplify_nd[]=
   {
      {u"Never" , u"Never simplify materials"}, // 0
      {u"Mobile", u"Simplify materials only when building applications for mobile platforms"}, // 1
      {u"Always", u"Always simplify materials"}, // 2
   }; ASSERT(MS_NEVER==0 && MS_MOBILE==1 && MS_ALWAYS==2);

   static cchar8 *text_data_modes[]=
   {
      "Disabled, Fast, Binary Only",
      "Enabled, Slower, Binary+Text",
   };

   static void CipherChanged          (ProjectSettings &ps) {Proj.cipher=(CIPHER_TYPE)ps.cipher_type(); Proj.cipher_time.getUTC(); CodeEdit.makeAuto(); Server.projectSetSettings(); ps.toGui(); if(Demo && ps.cipher_type())Gui.msgBox(S, "Encryption is not available in the demo version");}
   static void EncryptionKeyChanged   (ProjectSettings &ps) {Memc<Str> keys=Split(ps.cipher_key(), ','); if(keys.elms()>=Elms(Proj.cipher_key)){REPAO(Proj.cipher_key)=TextInt(keys[i]); Proj.cipher_key_time.getUTC(); CodeEdit.makeAuto(); Server.projectSetSettings();}}
   static void RandomizeEncryptionKey (ProjectSettings &ps) {REPAO(Proj.cipher_key)=Random(); Proj.cipher_key_time.getUTC(); CodeEdit.makeAuto(); Server.projectSetSettings(); ps.toGui();}
   static void CompressTypeChanged    (ProjectSettings &ps) {COMPRESS_TYPE type=ps.compressType(); Proj.compress_type    =type              ; Proj.compress_type_time    .getUTC(); Server.projectSetSettings(); ps.toGui(); if(Demo && type)Gui.msgBox(S, "Compression is not available in the demo version");}
   static void CompressLevelChanged   (ProjectSettings &ps) {                                      Proj.compress_level   =ps.compressLevel(); Proj.compress_level_time   .getUTC(); Server.projectSetSettings();}
   static void MaterialSimplifyChanged(ProjectSettings &ps) {                                      Proj.material_simplify=ps. mtrlSimplify(); Proj.material_simplify_time.getUTC(); Server.projectSetSettings();}
   static void TextDataChanged        (ProjectSettings &ps) {Proj.textData(ps.text_data()!=0); /*Server.projectSetSettings(); this is local user option*/}

   COMPRESS_TYPE compressType () {return InRange(compress_type(), cmpr_type) ? cmpr_type[compress_type()].type : COMPRESS_NONE;}
   int           compressLevel() {return compress_level();}

   MATERIAL_SIMPLIFY mtrlSimplify()C {return InRange(mtrl_simplify(), MS_NUM) ? MATERIAL_SIMPLIFY(mtrl_simplify()) : MS_NEVER;}

   void toGui()
   {
      VecI2 levels=CompressionLevels(Proj.compress_type);
      cipher_type.set(Proj.cipher, QUIET);
      cipher_key.visible(Proj.cipher!=CIPHER_NONE); randomize_key.visible(Proj.cipher!=CIPHER_NONE);
      Str s; FREPA(Proj.cipher_key){if(i)s+=", "; s+=Proj.cipher_key[i];} cipher_key.set(s, QUIET);
      REPA(cmpr_type)if(cmpr_type[i].type==Proj.compress_type){compress_type.set(i, QUIET); break;}
      compress_level.visible(levels.allDifferent());
      compress_level.setData(cmpr_lvl, Min(Elms(cmpr_lvl), levels.y+1));
      compress_level.set(Min(Proj.compress_level, compress_level.menu.list.elms()-1), QUIET);
      mtrl_simplify .set(Proj.material_simplify, QUIET);
      text_data.set(Proj.text_data, QUIET);
   }
   void create()
   {
      REPAO(cmpr_type).name=CompressionName(cmpr_type[i].type);
      ListColumn compress_lc[]=
      {
         ListColumn(MEMBER(CompressType, name), LCW_MAX_DATA_PARENT, "name"),
      };
      ListColumn name_desc_column[]=
      {
         ListColumn(MEMBER(NameDesc, name), LCW_MAX_DATA_PARENT, "Name"),
      };
      ListColumn class_name_desc_column[]=
      {
         ListColumn(MEMBER(ClassNameDesc, name), LCW_MAX_DATA_PARENT, "Name"),
      };
      Gui+=super.create(Rect_C(0, 0, 1.28, 0.47), "Project Settings").hide(); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      ts.reset().size=0.043; ts.align.set(1, 0);
      flt y=-0.05, h=0.05, p=0.07;
      Str cipher_desc; FREP(CIPHER_NUM){if(i)cipher_desc+="\n\n"; cipher_desc+=S+CipherText[i].name+" - "+CipherText[i].desc;}
      T+=cipher_type.create(Rect_L(0.01, y, 0.29, h)).func(CipherChanged, T).desc(cipher_desc); cipher_type.setColumns(class_name_desc_column, Elms(class_name_desc_column), true).setData(CipherText, CIPHER_NUM).set(0, QUIET).menu.list.setElmDesc(MEMBER(ClassNameDesc, desc));
      T+=cipher_key.create(Rect_L(0.32, y, 0.73, h)).func(EncryptionKeyChanged, T).desc("256 Encryption key numbers in \"X, X, X, ..\" format");
      T+=randomize_key.create(Rect_LU(cipher_key.rect().ru(), 0.21, cipher_key.rect().h()), "Randomize").func(RandomizeEncryptionKey, T); y-=p;
      T+=compress.create(Vec2(0.015, y), "Compression", &ts);
      T+=compress_type.create(Rect_L(0.32, y, 0.2, h)).func(CompressTypeChanged, T).desc("Select the compression type for files inside Pak.\n\nNo Compression offers biggest Pak size (100%), and best performance (26x) for accessing files in Pak."/*\nSNAPPY Compression offers big Pak size (~72%), and good performance (11x) for accessing files in Pak.*/"\nLZ4 Compression offers medium Pak size (~70%), and good performance (16x) for accessing files in Pak.\nZSTD Compression offers small Pak size (~63%), and medium performance (8x) for accessing files in Pak."/*\nZLIB Compression offers small Pak size (~64%), and medium performance (5x) for accessing files in Pak.*/"\nLZHAM Compression offers smaller Pak size (~57%), and slow performance (2x) for accessing files in Pak.\nLZMA Compression offers smallest Pak size (~55%), and slowest performance (1x) for accessing files in Pak.");
      compress_type.setColumns(compress_lc, Elms(compress_lc), true).setData(cmpr_type, Elms(cmpr_type)).set(0, QUIET).menu.list.setElmDesc(MEMBER(CompressType, desc));
      T+=compress_level.create(Rect_LU(compress_type.rect().ru(), 0.1, compress_type.rect().h()), cmpr_lvl, Elms(cmpr_lvl)).set(9, QUIET).func(CompressLevelChanged, T).desc("Compression level\n0=Fastest compression but biggest size\n..\n22=Slowest compression but smallest size"); y-=p;
      T+=mtrl_simplify_t.create(Vec2(0.015, y), "Simplify Materials", &ts);
      T+=mtrl_simplify  .create(Rect_L(0.32, y, 0.2, h)).func(MaterialSimplifyChanged, T).desc("Select if materials should be simplified when published.\nSimplifying material means removing its Normal, Bump, Specular and Glow maps, and keeping only Color and Alpha maps.\nThis can be used on Mobile platforms which typically don't make use of normal maps anyway.\nEnabling this option reduces package size of the application (because fewer textures are included).\nWhen material is simplified its normal map is removed, but before that, it's baked onto the color map, improving quality of the material.");
      mtrl_simplify.setColumns(name_desc_column, Elms(name_desc_column), true).setData(mtrl_simplify_nd, Elms(mtrl_simplify_nd)).set(0, QUIET).menu.list.setElmDesc(MEMBER(NameDesc, desc)); y-=p;
      T+=text_data_t.create(Vec2(0.015, y), "SVN Compatibility", &ts);
      T+=text_data.create(Rect_L(0.32, y, 0.52, h), text_data_modes, Elms(text_data_modes)).func(TextDataChanged, T).desc("This option allows to enable SVN Compatibility.\n\nBy default, Project Data can be synchronized across multiple computers only by using Esenthel Server application.\nWith this option enabled, you can synchronize your Project by using SVN as well.\n\nDisabled - Project Data is stored in Binary format only, which makes Project saving/loading faster, however SVN synchronization unavailable.\n\nEnabled - Project Data is stored in Binary and Text format, which makes Project saving/loading slower, however SVN synchronization possible.\nBinary version is used only as backup, in case the Text file would get messed up and is unable to be read from.\nHowever if the Text file is OK, then it will take precendence over Binary file, and Project Data will be read from the Text file and not the Binary file."); y-=p;
      T+=ok.create(Rect_D(clientWidth()/2, -clientHeight()+0.03, 0.3, 0.06), "OK").func(HideProjAct, SCAST(GuiObj, T));
   }
   void display()
   {
      activate();
      ok.kbSet();
   }
}
ProjectSettings ProjSettings;
/******************************************************************************/
