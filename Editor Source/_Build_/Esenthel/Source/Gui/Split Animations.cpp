/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
SplitAnimation SplitAnim;
/******************************************************************************/

/******************************************************************************/
      void SplitAnimation::Anim::Remove(Anim &anim) {SplitAnim.anims.removeData(&anim, true); SplitAnim.setList();}
      SplitAnimation::Anim::Anim()
      {
         SplitAnim.region+=remove.create().func(Remove, T).focusable(false).desc("Remove this split"); remove.image="Gui/close.img";
         SplitAnim.region+=name.create().desc("Name"          );
         SplitAnim.region+=from.create().desc("Start KeyFrame");
         SplitAnim.region+=to  .create().desc(  "End KeyFrame");
         SplitAnim.region+=loop.create().desc("Loop Animation");
      }
      void SplitAnimation::Anim::pos(flt y)
      {
         flt h=0.05f, total_width=SplitAnim.region.rect().w()-SplitAnim.region.slidebarSize(), frame_width=h*4;
         remove.rect(Rect_LU(Vec2(0, y), h));
         name  .rect(Rect_LU(remove.rect().ru(), total_width-h-frame_width*2-h, h));
         from  .rect(Rect_LU(name  .rect().ru(), frame_width, h));
         to    .rect(Rect_LU(from  .rect().ru(), frame_width, h));
         loop  .rect(Rect_LU(to    .rect().ru(), h          , h));
      }
   void SplitAnimation::Clipboard(SplitAnimation &sa) {sa.add(ClipGet());}
   void SplitAnimation::Split(SplitAnimation &sa) {sa.splitDo();}
   void SplitAnimation::New(SplitAnimation &sa) {sa.addNew();}
   void SplitAnimation::Clear(SplitAnimation &sa) {sa.clearDo();}
   void SplitAnimation::Hide(SplitAnimation &sa) {sa.hide(); if(AnimEdit.preview.visible())AnimEdit.preview.activate();else Proj.list.kbSet();}
   void SplitAnimation::splitDo()
   {
      if(anims.elms())
      {
         // verify
         REPA(anims)
         {
          C Anim &anim=anims[i];
            if(!anim.from().is() || !anim.to().is() || TextInt(anim.from())>TextInt(anim.to()))
            {
               Gui.msgBox(S, S+"Invalid KeyFrame range for \""+anim.name()+'"');
               return;
            }
         }
         
         // split
         if(Elm *base=Proj.findElm(anim_id, ELM_ANIM))if(ElmAnim *base_data=base->animData())
         {
            Mems<Edit::FileParams> files=Edit::FileParams::Decode(base->srcFile());
            if(files.elms()!=1)Gui.msgBox(S, S+"Unknown source file for animation.");else
            {
               Str src=files[0].name, anim_name; if(C TextParam *name=files[0].findParam("name"))anim_name=name->value;
               if(FileInfoSystem(src).type!=FSTD_FILE)Gui.msgBox(S, S+"Can't find file \""+src+'"');else
               {
                  TimeStamp time; time.getUTC();
                                    files[0].params.clear().setNum(3+anim_name.is());
                                    files[0].params[0].name="start_frame";
                                    files[0].params[1].name=  "end_frame";
                                    files[0].params[2].name="loop";
                  if(anim_name.is())files[0].params[3].set( "name", anim_name);
                  FREPA(anims)
                  {
                   C Anim &anim=anims[i];

                     // set only values, because names are the same for all animations
                     files[0].params[0].value=TextInt(anim.from());
                     files[0].params[1].value=TextInt(anim.to  ());
                     files[0].params[2].value=        anim.loop() ;

                     if(!i) // first animation just updates the existing one
                     {
                        base->setName(anim.name(), time).setSrcFile(Edit::FileParams::Encode(files), time); Proj.elmReload(base->id, false, false);
                        Server.renameElm(*base); Server.setElmShort(base->id);
                     }else // next are created as new anim elements
                     if(Elm *anim_elm=Proj.newElm(ELM_ANIM, base->parent_id, &anim.name(), false))if(ElmAnim *anim_data=anim_elm->animData())
                     {
                        anim_data->newVer();
                        anim_data->skel_id  =base_data->skel_id  ; anim_data->skel_time=time;
                        anim_data->transform=base_data->transform;
                        anim_data->linear   (base_data->linear()); // set linear to the same value as the main animation, this will make reload use that value
                        anim_data->setSrcFile(Edit::FileParams::Encode(files), time); Proj.elmReload(anim_elm->id, false, false);
                        // sending to Server was already queued in 'Proj.newElm'
                     }
                  }
                  Proj.refresh(); // names were changed, elements added and importing set
                  hide();
               }
            }
         }
      }
   }
   void SplitAnimation::clearDo() {anims.clear(); setList();}
   void SplitAnimation::addNew() {anims.New  (); setList();}
   void SplitAnimation::setList()
   {
      flt y=0; FREPA(anims)
      {
         Anim &anim=anims[i]; anim.pos(y); y-=anim.remove.size().y;
      }
   }
   void SplitAnimation::create()
   {
      Gui+=::EE::Window::create(Rect_C (0, 0, 1.3f, 1.15f)); button[2].show().func(Hide, T);
      T+=text     .create(Rect_C (clientWidth()/2   , -0.07f, clientWidth()-0.04f, 0), "Drag and drop a file with animations list on this window,\nor use the button to detect them from clipboard."); text.auto_line=AUTO_LINE_SPACE_SPLIT;
      T+=clear    .create(Rect_LU(              0.04f, -0.15f, 0.2f, 0.06f), "Clear"                ).func(Clear    , T);
      T+=clipboard.create(Rect_U (clientWidth()/2   , -0.15f, 0.5f, 0.06f), "Detect from Clipboard").func(Clipboard, T);
      T+=add_new  .create(Rect_RU(clientWidth()-0.04f, -0.15f, 0.2f, 0.06f), "New"                  ).func(New      , T);
      T+=t_name   .create(Vec2   (              0.45f, -0.255f), "Name");
      T+=t_start  .create(Vec2   (clientWidth()-0.44f, -0.255f), "Start");
      T+=t_end    .create(Vec2   (clientWidth()-0.24f, -0.255f), "End");
      T+=t_loop   .create(Vec2   (clientWidth()-0.12f, -0.255f), "Loop");
      T+=split    .create(Rect_D (clientWidth()/2, -clientHeight()+0.04f, 0.3f, 0.06f), "Split").func(Split, T);
      T+=region   .create(Rect   (clear.rect().min.x, split.rect().max.y+0.04f, add_new.rect().max.x, t_name.rect().min.y-0.02f));
   }
   void SplitAnimation::activate(C UID &elm_id)
   {
      if(Elm *elm=Proj.findElm(elm_id, ELM_ANIM))
      {
         anim_id=elm_id;
         ::EE::GuiObj::activate();
         ::EE::Window::title=S+"Split Animation \""+elm->name+'"';
         anims.clear();
         setList();
      }
   }
   bool SplitAnimation::Create(int &data, C Str &key, ptr user) {data=0; return true;}
   bool SplitAnimation::IsNumber(C Str &str) {return FlagTest(CharFlag(str[0]), CHARF_DIG);}
   void SplitAnimation::add(C Str &text)
   {
      // auto-detect from following samples: (watch out as names may include numbers)
      
      // frame 6-65:name
      // standby,15,55
      // Combat Mode C 604-680
      // idle 0--50
      // stand: 0 - 40
      // 1-200 Idle

      // split into lines
      Memt<Str> lines; ::Split(lines, text, '\n');

      // first find what tokens occur the most between 2 numbers
      Map<Str, int> separators(CompareCI, Create), left(CompareCI, Create), right(CompareCI, Create);
      FREPA(lines)
      {
         Memc<Str> tokens; Tokenize(tokens, lines[i]); if(tokens.elms())
         {
            int number_pos=-1; FREPA(tokens)if(IsNumber(tokens[i]))
            {
               if(number_pos>=0) // if there was a number detected
               {
                  Str separator; for(int j=number_pos+1; j<i; j++)separator.space()+=tokens[j];
                  if(int *s=separators(separator))(*s)++; // increase occurence of this separator
               }
               number_pos=i;
            }
            if(!IsNumber(tokens.first()))if(int *s=left (tokens.first()))(*s)++; // increase occurence of first token
            if(!IsNumber(tokens.last ()))if(int *s=right(tokens.last ()))(*s)++; // increase occurence of last  token
         }
      }

      // find the separator with most occurences
    C Str *separator=null; int occurences=0;
      REPA(separators)if(separators[i]>occurences)
      {
         occurences= separators    [i];
         separator =&separators.key(i);
      }

      // find which side repeats more
      int left_repeat=0, right_repeat=0;
      REPA( left)MAX( left_repeat,  left[i]);
      REPA(right)MAX(right_repeat, right[i]);

      // tokenize the separator
      Memt<Str> sep_tokens; if(separator)Tokenize(sep_tokens, *separator);

      // process all lines and look for "number, sep_tokens, number"
      FREPA(lines)
      {
         Memc<Str> tokens; Tokenize(tokens, lines[i]);
         FREPA(tokens)if(IsNumber(tokens[i]) && InRange(i+1+sep_tokens.elms(), tokens) && IsNumber(tokens[i+1+sep_tokens.elms()]))
         {
            for(int j=0; j<sep_tokens.elms(); j++)if(!InRange(i+1+j, tokens) || tokens[i+1+j]!=sep_tokens[j])goto different;
            {
               // set name ranges (inclusive)
               VecI2 name_range_left (0                      ,             i-1),
                     name_range_right(i+1+sep_tokens.elms()+1, tokens.elms()-1);

               // remove symbol token next to frame ranges
               if(InRange(name_range_left .y, tokens) && CharType(tokens[name_range_left .y][0])==CHART_SIGN)name_range_left .y--;
               if(InRange(name_range_right.x, tokens) && CharType(tokens[name_range_right.x][0])==CHART_SIGN)name_range_right.x++;
               
               bool left=(left_repeat<right_repeat);
               if(name_range_left .y<name_range_left .x)left=false;else // there are no name tokens on the left  side
               if(name_range_right.y<name_range_right.x)left=true ;     // there are no name tokens on the right side

               VecI2 name_range=(left ? name_range_left : name_range_right);
               Str name; for(int i=name_range.x; i<=name_range.y; i++)if(InRange(i, tokens))name.space()+=tokens[i];

               Anim &anim=anims.New();
               anim.from.set(S+TextInt(tokens[i]));
               anim.to  .set(S+TextInt(tokens[i+1+sep_tokens.elms()]));
               anim.name.set(name.replace('_', ' '));
               anim.loop.set(Contains(name, "loop", false, true) || Contains(name, "looped", false, true));
               break; // stop looking
            }
         different:; // keep looking
         }
      }
      setList();
   }
   void SplitAnimation::drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(contains(obj))
      {
         FREPA(names)
         {
          C Str &name=names[i], ext=GetExt(name);
            if(ext=="txt")
            {
               FileText f; if(f.read(name)){add(f.getAll()); break;}
            }else
            if(ext=="meta") // unity metafile
            {
               bool added=false;
               TextData td;
               if(td.loadYAML(name))
               if(C TextNode *ModelImporter=td.findNode("ModelImporter"))
               if(C TextNode *animations=ModelImporter->findNode("animations"))
               if(C TextNode *clipAnimations=animations->findNode("clipAnimations"))
               FREPA(clipAnimations->nodes)
               {
                C TextNode &clip=clipAnimations->nodes[i];
                  Str name, first, last;
                  if(C TextNode *n=clip.findNode(      "name"))name =n->asText();
                  if(C TextNode *n=clip.findNode("firstFrame"))first=n->asText();
                  if(C TextNode *n=clip.findNode( "lastFrame"))last =n->asText();
                  if(name.is() || first.is() || last.is())
                  {
                     Anim &anim=anims.New();
                     anim.from.set(first);
                     anim.to  .set(last );
                     anim.name.set(name );
                     if(C TextNode *n=clip.findNode("loop"))anim.loop.set(n->asBool());
                     added=true;
                  }
               }
               if(added)setList();
            }
         }
         names.clear();
      }
   }
SplitAnimation::SplitAnimation() : anim_id(UIDZero) {}

/******************************************************************************/
