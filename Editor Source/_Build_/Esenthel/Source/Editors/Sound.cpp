/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
SoundEditor SoundEdit;
/******************************************************************************/

/******************************************************************************/
   ::SoundEditor::ImportAs SoundEditor::Import_as[]=
   {
      {"Original", SOUND_NONE, -1},
      {"Vorbis, auto BitRate", SOUND_SND_VORBIS, 0},
      {"Opus, auto BitRate", SOUND_SND_OPUS, 0},
      {"Uncompressed", SOUND_WAV, -1},
      {null, SOUND_NONE, 0},
    //{"Original, 32 Kbps relative BitRate", SOUND_NONE, 32}, Vorbis and Opus disabled
      {"Original, 48 Kbps relative BitRate", SOUND_NONE, 48},
      {"Original, 64 Kbps relative BitRate", SOUND_NONE, 64},
      {"Original, 80 Kbps relative BitRate", SOUND_NONE, 80},
      {"Original, 96 Kbps relative BitRate", SOUND_NONE, 96},
      {"Original, 112 Kbps relative BitRate", SOUND_NONE, 112},
      {"Original, 128 Kbps relative BitRate", SOUND_NONE, 128},
      {null, SOUND_NONE, 0},
      {"Vorbis, original BitRate", SOUND_SND_VORBIS, -1},
    //{"Vorbis, 32 Kbps relative BitRate", SOUND_SND_VORBIS, 32}, // only aoTuV
      {"Vorbis, 48 Kbps relative BitRate", SOUND_SND_VORBIS, 48},
      {"Vorbis, 64 Kbps relative BitRate", SOUND_SND_VORBIS, 64},
      {"Vorbis, 80 Kbps relative BitRate", SOUND_SND_VORBIS, 80},
      {"Vorbis, 96 Kbps relative BitRate", SOUND_SND_VORBIS, 96},
      {"Vorbis, 112 Kbps relative BitRate", SOUND_SND_VORBIS, 112},
      {"Vorbis, 128 Kbps relative BitRate", SOUND_SND_VORBIS, 128},
      {null, SOUND_NONE, 0},
      {"Opus, original BitRate", SOUND_SND_OPUS, -1},
    //{"Opus, 32 Kbps relative BitRate", SOUND_SND_OPUS, 32}, very bad quality
      {"Opus, 48 Kbps relative BitRate", SOUND_SND_OPUS, 48},
      {"Opus, 64 Kbps relative BitRate", SOUND_SND_OPUS, 64},
      {"Opus, 80 Kbps relative BitRate", SOUND_SND_OPUS, 80},
      {"Opus, 96 Kbps relative BitRate", SOUND_SND_OPUS, 96},
      {"Opus, 112 Kbps relative BitRate", SOUND_SND_OPUS, 112},
      {"Opus, 128 Kbps relative BitRate", SOUND_SND_OPUS, 128},
   };
   int SoundEditor::ImportAsElms=Elms(Import_as);
/******************************************************************************/
   void SoundEditor::Locate(SoundEditor &editor) {Proj.elmLocate(editor.elm_id);}
   void SoundEditor::Seek(SoundEditor &editor) {editor.seek_pos=editor.progress(); editor.seek_set=true;}
   void SoundEditor::Play(SoundEditor &editor)
   {
      if(editor.sound.playing())editor.sound.stop();else
      {
         if(!editor.sound.size() && editor.elm)editor.sound.create(Proj.gamePath(*editor.elm), editor.loop, editor.volume);
         if(editor.seek_set){editor.seek_set=false; editor.sound.frac(editor.seek_pos);}
         editor.sound.play();
      }
   }
   void SoundEditor::SetImportAs(SoundEditor &editor, C Str &t)
   {
      int i=TextInt(t); if(InRange(i, Import_as))
      {
       C ImportAs &ia=Import_as[i];
         Proj.soundImportAs(editor.elm_id, ia.codec, ia.bit_rate);
      }
   }
   Str SoundEditor::GetImportAs(C SoundEditor &editor)
   {
      if(editor.elm)
      {
         Mems<Edit::FileParams> files=Edit::FileParams::Decode(editor.elm->srcFile());
         if(files.elms())
         {
            SOUND_CODEC codec=SOUND_NONE; if(C TextParam *param=files[0].findParam("codec"))codec=TextSoundCodec(param->value);
            int         rel_bit_rate=-1 ; C TextParam *rbr=files[0].findParam("relBitRate"); if(!rbr)rbr=files[0].findParam("relativeBitRate"); if(rbr)rel_bit_rate=rbr->asInt();
            int         best=-1, bit_rate_d=0;
            REPA(Import_as)
            {
             C ImportAs &ia=Import_as[i]; if(ia.name && ia.codec==codec && Sign(rel_bit_rate)==Sign(ia.bit_rate))
               {
                  int d=Abs(ia.bit_rate-rel_bit_rate);
                  if(best<0 || d<bit_rate_d){best=i; bit_rate_d=d;}
               }
            }
            return best;
         }
      }
      return S;
   }
   void SoundEditor::ApplyVol(SoundEditor &editor)
   {
      if(editor.elm && editor.vol)
      {
         flt volume=TextFlt(editor.vol->textline()); // use textline instead of editor.volume because that one is clamped to 0..1
         Proj.mulSoundVolume(editor.elm_id, volume);
         editor.vol->set(1);
      }
   }
   void SoundEditor::create()
   {
      import_as=&add("Import as", MemberDesc().setFunc(GetImportAs, SetImportAs)).setEnum();
   #if 1 // use menu to disable clicking on null strings
      Node<MenuElm> n; FREPA(Import_as)n.New().create(Import_as[i].name);
      import_as->combobox.setData(n);
   #else
      ListColumn lc[]=
      {
         ListColumn(MEMBER(ImportAs, name), LCW_MAX_DATA_PARENT, S),
      };
      import_as.combobox.setColumns(lc, Elms(lc)).setData(Import_as, Elms(Import_as));
   #endif
      length  =&add();
      channels=&add();
      freq    =&add();
      kbps    =&add();
      size    =&add();
      codec   =&add();
                add("Loop"  , MemberDesc(MEMBER(SoundEditor, loop)));
      vol     =&add("Volume", MemberDesc(MEMBER(SoundEditor, volume))).range(0, 1);
      autoData(this);

      Rect r=::PropWin::create("Sound Player", Vec2(0.02f, -0.02f), 0.036f, 0.043f, 0.15f); button[2].func(HideProjAct, SCAST(GuiObj, T)).show();
      import_as->combobox.resize(Vec2(0.33f, 0));
      rect(Rect_C(0, 0, 0.67f, 0.481f));
      r.set(r.max.x, -clientHeight(), clientWidth(), 0); r.extend(-0.04f);
      T+=play     .create(Rect_U(r.up()+Vec2(0, -0.05f), 0.23f, 0.055f)).func(Play, T);
      T+=progress .create(Rect_U(play    .rect().down()-Vec2(0, 0.030f), r.w(), 0.06f)).func(Seek, T);
      T+=locate   .create(Rect_U(progress.rect().down()-Vec2(0, 0.030f), 0.15f, 0.055f), "Locate").func(Locate, T).desc("Locate this element in the Project");
      T+=apply_vol.create(Rect_LU(vol->button.rect().ru()+Vec2(vol->button.rect().h(), 0), 0.11f, vol->button.rect().h()), "Apply").func(ApplyVol, T).focusable(false).desc("Apply volume scale on the sound file");
   }
   void SoundEditor::update(bool set_frac)
   {
      if(set_frac)progress.set(sound.frac(), QUIET);
      play.text=(sound.playing() ? "Stop" : "Play");
      sound.loop(loop).volume(volume);
   }
   void SoundEditor::setInfo()
   {
      toGui();
      if(length  )length  ->name.set(S+"Length: "   +TextReal(sound.length(), -2)+'s');
      if(channels)channels->name.set(S+"Channels: " +((sound.channels()==1) ? S+"Mono" : (sound.channels()==2) ? S+"Stereo" : S+sound.channels()));
      if(freq    )freq    ->name.set(S+"Frequency: "+sound.frequency());
      if(kbps    )kbps    ->name.set(S+"Kbps: "     +DivRound(sound.bitRate(), 1000));
      if(size    )size    ->name.set(S+"Size: "     +FileSize(FSize(sound.name())));
      if(codec   )codec   ->name.set(S+"Codec: "    +sound.codecName());
   }
   void SoundEditor::update(C GuiPC &gpc)
{
      if(seek_set && sound.size())
      {
         REPA(MT)if(MT.b(i) && MT.guiObj(i)==&progress)goto setting; // check if there's still an input editing the progress
         seek_set=false; sound.frac(seek_pos);
      setting:;
      }
      if(gpc.visible && visible())update(!seek_set);
      ::EE::ClosableWindow::update(gpc); // call after 'update' so progress value will be recent once 'Seek' is called
   }
   void SoundEditor::draw(C GuiPC &gpc)
{
      ::EE::Window::draw(gpc);
      if(elm && import_as->combobox.contains(Gui.ms()) && Proj.invalidSrc(elm->srcFile())) // if source was not found, then draw exclamation
      {
         D.clip(gpc.clip);
         Proj.exclamation->drawFit(Rect_LD(import_as->combobox.rect().min+clientRect().lu()+gpc.offset, import_as->combobox.rect().h()));
      }
   }
   SoundEditor& SoundEditor::hide(){set(null); ::PropWin::hide(); return T;}
   void SoundEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmSound *data=elm->soundData())data->newVer(); // modify just before saving/sending in case we've received data from server after edit
         Preview.elmChanged(elm->id);
      }
      changed=false;
   }
   void SoundEditor::setChanged()
   {
      if(elm)
      {
         changed=true;
         if(ElmSound *data=elm->soundData())data->newVer();
      }
   }
   void SoundEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_SOUND)elm=null;
      if(T.elm!=elm)
      {
         flush();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         bool play=sound.playing();
         sound.close(); if(elm)sound.create(Proj.gamePath(*elm), loop, volume); // always close to wait for file handle release
         if(play)sound.play();
         update(); setInfo();
         Proj.refresh(false, false);
         visible(T.elm!=null).moveToTop();
      }
   }
   void SoundEditor::activate(Elm *elm) {set(elm); if(T.elm)::EE::GuiObj::activate();}
   void SoundEditor::toggle(Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void SoundEditor::closeElm(C UID &elm_id)
   {
      if(elm && elm->id==elm_id)
      {
         sound.close();
      }
   }
   void SoundEditor::elmChanged(C UID &elm_id)
   {
      if(elm && elm->id==elm_id)
      {
         sound.create(Proj.gamePath(*elm), loop, volume);
         if(play.text=="stop")sound.frac(progress()).play(); // if was playing
         update(); setInfo();
      }
   }
   void SoundEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
SoundEditor::SoundEditor() : elm_id(UIDZero), elm(null), changed(false), loop(false), seek_set(false), volume(1), seek_pos(0), import_as(null), length(null), channels(null), freq(null), kbps(null), size(null), codec(null), vol(null) {}

/******************************************************************************/
