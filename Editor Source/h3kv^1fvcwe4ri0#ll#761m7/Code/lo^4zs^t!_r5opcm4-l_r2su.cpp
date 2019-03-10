/******************************************************************************/
class ImporterClass
{
   enum MODE
   {
      UPDATE, // reload target from source file
      ANIM  , // import animations only to target
      CLOTH , // import as cloth to target
      ADD   , // add source to target
   }
   class Import
   {
      class MaterialEx : XMaterial
      {
         Material mtrl;
         Image    base_0, base_1, detail, macro, reflection, light;
         UID      base_0_id=UIDZero, base_1_id=UIDZero, detail_id=UIDZero, macro_id=UIDZero, reflection_id=UIDZero, light_id=UIDZero;

         void copyTo(EditMaterial &dest, C TimeStamp &time=TimeStamp().getUTC())C
         {
            dest.create(mtrl, time); // set from 'Material' instead of 'XMaterial' because it had '_adjustParams' called
            dest. flip_normal_y=    flip_normal_y; dest. flip_normal_y_time=time;
            dest.     color_map=        color_map; dest.     color_map_time=time;
            dest.     alpha_map=        alpha_map; dest.     alpha_map_time=time;
            dest.      bump_map=         bump_map; dest.      bump_map_time=time;
            dest.    normal_map=       normal_map; dest.    normal_map_time=time;
            dest.  specular_map=     specular_map; dest.  specular_map_time=time;
            dest.      glow_map=         glow_map; dest.      glow_map_time=time;
            dest.     light_map=        light_map; dest.     light_map_time=time;
            dest.reflection_map=   reflection_map; dest.reflection_map_time=time;
            dest.     macro_map=                S; dest.     macro_map_time=time;
            dest.detail_color  = detail_color_map; dest.    detail_map_time=time;
            dest.detail_bump   =  detail_bump_map;
            dest.detail_normal =detail_normal_map;
            dest.    base_0_tex=    base_0_id;
            dest.    base_1_tex=    base_1_id;
            dest.    detail_tex=    detail_id;
            dest.     macro_tex=     macro_id;
            dest.reflection_tex=reflection_id;
            dest.     light_tex=     light_id;
         }
         void check(C Str &path, Str &tex)
         {
            Mems<Edit.FileParams> texs=Edit.FileParams.Decode(tex);
            FREPA(texs)if(!FExistSystem(texs[i].name))
            {
               Str test=path; test.tailSlash(true)+=GetBaseNoExt(texs[i].name); test+=".webp";
               if(FExistSystem(test))texs[i].name=test;
            }
            tex=Edit.FileParams.Encode(texs);
         }
         void process(C Str &path)
         {
            copyParamsTo(mtrl);

            if(path.is())
            {
               check(path, color_map);
               check(path, alpha_map);
               check(path, bump_map);
               check(path, glow_map);
               check(path, light_map);
               check(path, normal_map);
               check(path, specular_map);
               check(path, reflection_map);
            }

            if(GetExt(color_map)=="img" || GetExt(normal_map)=="img" || GetExt(detail_color_map)=="img" || GetExt(reflection_map)=="img") // this is 'EE.Material' ("mtrl" format)
            {
               Str b0=color_map, b1=normal_map, d=detail_color_map, r=reflection_map, l=light_map, m;
               base_0    .load(b0); ImageProps(base_0    , &    base_0_id, null, ForceHQMtrlBase0 ? FORCE_HQ : 0);
               base_1    .load(b1); ImageProps(base_1    , &    base_1_id, null, ForceHQMtrlBase1 ? FORCE_HQ : 0);
               detail    .load(d ); ImageProps(detail    , &    detail_id, null, IGNORE_ALPHA);
               macro     .load(m ); ImageProps(macro     , &     macro_id, null, IGNORE_ALPHA);
               reflection.load(r ); ImageProps(reflection, &reflection_id, null, IGNORE_ALPHA);
               light     .load(l ); ImageProps(light     , &     light_id, null, IGNORE_ALPHA);

               Edit.FileParams fp; 
               fp=b0; if(fp.name.is())fp.getParam("channel").setValue("rgb"); color_map=fp.encode();
                  alpha_map.clear();
                   bump_map.clear();
                   glow_map.clear();
                 normal_map.clear();
               specular_map.clear();
               fp=d; if(fp.name.is())fp.getParam("channel").setValue("z" ); detail_color_map =fp.encode();
               fp=d; if(fp.name.is())fp.getParam("channel").setValue("w" ); detail_bump_map  =fp.encode();
               fp=d; if(fp.name.is())fp.getParam("channel").setValue("xy"); detail_normal_map=fp.encode();
               if(b1.is())
               {
                  fp=b0; if(fp.name.is())fp.getParam("channel").setValue("a" );     bump_map=fp.encode();
                  fp=b1;                 fp.getParam("channel").setValue("xy");   normal_map=fp.encode();
                  fp=b1;                 fp.getParam("channel").setValue("z" ); specular_map=fp.encode();
                  fp=b1;                 fp.getParam("channel").setValue("w" );    alpha_map=fp.encode();
                  fp=b1;                 fp.getParam("channel").setValue("w" );     glow_map=fp.encode();
               }else
               if(b0.is())
               {
                  fp=b0; fp.getParam("channel").setValue("a"); alpha_map=fp.encode();
               }
            }else
            {
               Image col, alpha, bump, normal, spec, glow;
               ImportImage(       col,      color_map);
               ImportImage(     alpha,      alpha_map);
               ImportImage(      bump,       bump_map);
               ImportImage(    normal,     normal_map);
               ImportImage(      spec,   specular_map);
               ImportImage(      glow,       glow_map);
               ImportImage(reflection, reflection_map);
               ImportImage(     light,      light_map);

               // process textures only if they're added for the first time, otherwise delete them so they won't be saved
               uint       bt=CreateBaseTextures(base_0, base_1, col, alpha, bump, normal, spec, glow, true, flip_normal_y);
               IMAGE_TYPE ct; ImageProps(    base_0, &    base_0_id, &ct, ForceHQMtrlBase0 ? FORCE_HQ : 0); if(Importer.includeTex(    base_0_id))                           base_0    .copyTry(base_0    , -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, false, false, false, false); else base_0    .del();
                              ImageProps(    base_1, &    base_1_id, &ct, ForceHQMtrlBase1 ? FORCE_HQ : 0); if(Importer.includeTex(    base_1_id))                           base_1    .copyTry(base_1    , -1, -1, -1, ct, IMAGE_2D, 0, FILTER_BEST, false, false, false, true ); else base_1    .del();
                              ImageProps(reflection, &reflection_id, &ct, IGNORE_ALPHA                   ); if(Importer.includeTex(reflection_id)){FixAlpha(reflection, ct); reflection.copyTry(reflection, -1, -1, -1, ct, IMAGE_CUBE                                          );}else reflection.del();
                              ImageProps(     light, &     light_id, &ct, IGNORE_ALPHA                   ); if(Importer.includeTex(     light_id)){FixAlpha(light     , ct); light     .copyTry(light     , -1, -1, -1, ct, IMAGE_2D, 0                                         );}else light     .del();
               mtrl._adjustParams(~bt, bt);
            }
         }
      }
      class ImageEx : Image
      {
         bool cube=false;
         File raw;
      }

      int              status=-1; // -1=in progress, 0=fail, 1=ok
      bool             has_loop=false, cancel=false, remember_result=false, has_color=true, has_alpha=true, ignore_anims=false;
      ELM_TYPE         type=ELM_NONE;
      MODE             mode=UPDATE;
      UID              elm_id=UIDZero, parent_id=UIDZero;
      Str              file, force_name;
      Mesh             mesh;
      Skeleton         skel;
      Memc<XAnimation> anims;
      Memc<MaterialEx> mtrls;
      Memc<int>        part_mtrl_index;
      Memc<Str>        bone_names;
      Memc<ImageEx>    images;
      File             raw;
      EditFont         edit_font;
      Font             font;
      Str              code;

      Str nodeName(int i)C {return InRange(i, bone_names) ? bone_names[i] : InRange(i, skel.bones) ? (Str)skel.bones[i].name : S;}
      Str nodeUID (int i)C // unique string identifying a node !! needs to be the same as 'EditSkeleton.nodeUID' !!
      {
         Str path; for(; InRange(i, skel.bones); )
         {
            Str node_name=nodeName(i);
            path+=node_name; // node name
            int parent=skel.boneParent(i), child_index=0; REPD(j, i)if(skel.boneParent(j)==parent && nodeName(j)==node_name)child_index++; if(child_index){path+=CharAlpha; path+=child_index;} // node child index in parent (only children with same names are counted)
            path+='/'; // separator
            i=parent;
         }
         return path;
      }

      Import& set(C UID &elm_id, C UID &parent_id, C Str &file, MODE mode, ELM_TYPE type, C Str &force_name, bool remember_result)
      {
         T.elm_id=elm_id; T.parent_id=parent_id; T.file=file; T.mode=mode; T.type=type; T.force_name=force_name; T.remember_result=remember_result;
         switch(type)
         {
            case ELM_FONT: edit_font.load(Proj.editPath(elm_id)); break;

            case ELM_IMAGE: // check for skybox
            {
               Mems<Edit.FileParams> files=Edit.FileParams.Decode(T.file);
               if(files.elms()==6)
               {
                  // load helper data from the project
                  File src;
                  UID  image_id;
                  REPA(files)if(DecodeFileName(files[i].name, image_id))if(src.readTry(Proj.editPath(image_id)))
                  {
                     ImageEx &image=images(i);
                     src.copy(image.raw.writeMem());
                     if(Elm *elm=Proj.findElm(image_id))if(ElmImage *data=elm.imageData())image.cube=(data.mode==IMAGE_CUBE);
                  }

                  // check if any of the images were not imported
                  if(src.readTry(Proj.editPath(elm_id)))src.copy(raw.writeMem());
               }
            }break;

            case ELM_OBJ: // check if we want to ignore animations
            {
               if(mode==UPDATE // check only for UPDATE because others treat 'elm_id' as target
               && Proj.objToSkel(elm_id).valid())ignore_anims=true; // if object that's being reloaded already had a skeleton set, then ignore loading animations because they were already imported before
            }break;
         }
         return T;
      }

      static bool ApplyVolume(ptr data, int size, flt vol)
      {
         if(data)
         {
            if(size&1)return false; // need 2 byte alignment for a full sample
            short *sample=(short*)data;
            REP(size/SIZE(short))
            {
              *sample=Mid(Round(*sample * vol), SHORT_MIN, SHORT_MAX);
               sample++;
            }
         }
         return true;
      }

      bool import() // !! this is called on a secondary thread !!
      {
         Mems<Edit.FileParams> files=Edit.FileParams.Decode(T.file);
         Str file; if(files.elms())file=files[0].name;
         bool all_nodes_as_bones=(type==ELM_ANIM || mode==ANIM); // when importing animations we have to treat all nodes as potential bones, because it's possible that asset developer used helper/dummies to animate nodes which in the base object mesh model got imported as bones due to mesh skinning, however animations don't need the mesh and without the mesh and thus skinning, the nodes would not get detected as bones, to workaround this we force all nodes as bones, and later we just remove them depending if they're present in the already existing object mesh skeleton from before, removing happens in 'removeExtraBones' which is called in processing the animation later.

         // import
         switch(type)
         {
            case ELM_OBJ:
            {
               MemPtr<XAnimation> anims; if(mode!=CLOTH && mode!=ADD && !ignore_anims)anims.point(T.anims);
               MemPtr<XMaterial > mtrls; if(mode!=ANIM                               )mtrls.point(T.mtrls);
               if(EE.Import(file, (mode!=ANIM) ? &mesh : null, &skel, anims, mtrls, part_mtrl_index, bone_names, all_nodes_as_bones))
               {
                  FixMesh(mesh);
                  REPAO(anims).anim.linear(anims[i].fps>=LinearAnimFpsLimit).clip(0, anims[i].anim.length()); // set linear mode and remove any keyframes outside of anim range
                  Str path=GetPath(file); FREPAO(T.mtrls).process(path);
                  return true;
               }
            }break;

            case ELM_SOUND:
            {
               SOUND_CODEC codec=SOUND_NONE; if(C TextParam *param=files[0].findParam("codec" ))codec =TextSoundCodec(param.value);
               dbl         start=0;          if(C TextParam *param=files[0].findParam("start" ))start =param.asDbl();
               dbl         end   =-1;        if(C TextParam *param=files[0].findParam("end"   ))end   =param.asDbl();
               dbl         length=-1;        if(C TextParam *param=files[0].findParam("length"))length=param.asDbl();
               flt         volume=1;         if(C TextParam *param=files[0].findParam("volume"))volume=param.asFlt(); bool apply_vol=!Equal(volume, 1);
               int         hz=-1;            if(C TextParam *param=files[0].findParam("hz"    ))hz    =param.asInt();
               int         rel_bit_rate=-1 ; C TextParam *rbr=files[0].findParam("relBitRate"); if(!rbr)rbr=files[0].findParam("relativeBitRate"); if(rbr)rel_bit_rate=rbr.asInt();
               if(codec || rel_bit_rate>=0 || start>0 || end>=0 || length>=0 || hz>0 || apply_vol)
               {
                  SoundStream s; if(s.create(file) && s.block())
                  {
                     long  start_sample=((start> 0) ? RoundL(start*s.frequency()) :           0); MIN(start_sample, s.samples());
                     long    end_sample=((end  >=0) ? RoundL(end  *s.frequency()) : s.samples()); MIN(  end_sample, s.samples());
                     long length_sample=Max(0, end_sample-start_sample); if(length>=0)MIN(length_sample, RoundL(length*s.frequency()));
                     if(!codec)codec=s.codec(); // if not specified then use original
                     if(hz<=0)hz=s.frequency();
                     
                     int bit_rate=-1; if(rel_bit_rate>0) // calculate from relative bit rate
                     {
                        bit_rate=long(rel_bit_rate*1000) * s.frequency()/44100 * s.channels()/2;
                     }else
                     if(rel_bit_rate<0 && (Equal(s.codecName(), "opus") || Equal(s.codecName(), "vorbis"))) // get from source (accept only from good lossy compressed codecs, because Raw/Flac will have very big values)
                     {
                        bit_rate=s.bitRate();
                     }
                     if(codec && s.codecName()!=CodecName(codec) // compare names instead of value, to ignore audio container and just check codec, because names always return just the codec
                     || bit_rate>0 && Abs(bit_rate-s.bitRate())>8*1000 // allow 8 kbit tolerance
                     || start_sample>0 // if we want to skip some data
                     || length_sample!=s.samples() // if we want to skip some data
                     || apply_vol // if we want to change volume
                     || hz!=s.frequency() // want to change frequency
                     )
                     {
                        if(codec!=SOUND_WAV
                        && !Equal(CodecName(codec), "opus")
                        && !Equal(CodecName(codec), "vorbis")
                        )codec=SOUND_SND_OPUS; // if selected codec is not supported then use a default one

                        if(bit_rate<=0) // use default bit rate
                        {
                           if(Equal(CodecName(codec), "opus"  ))bit_rate=long(DefaultOpusBitRate  ) * s.frequency()/44100 * s.channels()/2;else
                           if(Equal(CodecName(codec), "vorbis"))bit_rate=long(DefaultVorbisBitRate) * s.frequency()/44100 * s.channels()/2;
                        }
                        int bit_rate_hq=Max(long(DefaultHQBitRate) * s.frequency()/44100 * s.channels()/2, bit_rate);

                        raw.writeMem();
                        byte temp[65536];
                        if(!s.pos(start_sample*s.block()))goto error;
                        long size=length_sample*s.block();
                        if(codec==SOUND_WAV)
                        {
                           SndRawEncoder encoder; if(encoder.create(raw, hz, s.channels(), length_sample))
                           {
                              for(; size>0; )
                              {
                                 int r=s.set(temp, Min(size, (int)SIZE(temp))); if(r<=0)goto error;
                                 if(apply_vol && !ApplyVolume(temp, r, volume))goto error;
                                 if(!encoder.encode(temp, r))goto error; size-=r;
                              }
                              if(!encoder.finish())goto error; return true;
                           }
                        }else
                        if(Equal(CodecName(codec), "opus"))
                        {
                           SndOpusEncoder encoder; if(encoder.create(raw, length_sample, hz, s.channels(), bit_rate_hq)) // prioritize first frame (to avoid pop/clicks for looped audio)
                           {
                              int  frame_size=encoder.block()*encoder.frameSamples();
                              long raw_start=raw.pos();
                              bool first=true;
                              for(; size>0; )
                              {
                                 int r=Min(size, (int)SIZE(temp));
                                 if(first)MIN(r, frame_size); // make sure we encode only the first frame with higher bit rate
                                 r=s.set(temp, r);
                                 if(r<=0)goto error;
                                 if(apply_vol && !ApplyVolume(temp, r, volume))goto error;
                                 if(!encoder.encode(temp, r))goto error; size-=r;
                                 if(first && raw.pos()!=raw_start) // if wrote the first frame
                                 {
                                    first=false;
                                    encoder.bitRate(bit_rate); // set actual desired bit rate
                                 }
                              }
                              encoder.bitRate(bit_rate_hq); // prioritize last frame (to avoid pop/clicks for looped audio)
                              if(!encoder.finish())goto error; return true;
                           }
                        }else
                        if(Equal(CodecName(codec), "vorbis") && s.channels() && s.frequency())
                        {
                           rel_bit_rate=long(bit_rate) * 2/s.channels() * 44100/s.frequency();
                           flt quality=VorbisBitRateToQuality(rel_bit_rate);
                           OggVorbisEncoder encoder; if(encoder.create(raw, hz, s.channels(), quality))
                           {
                              for(; size>0; )
                              {
                                 int r=s.set(temp, Min(size, (int)SIZE(temp))); if(r<=0)goto error;
                                 if(apply_vol && !ApplyVolume(temp, r, volume))goto error;
                                 if(!encoder.encode(temp, r))goto error; size-=r;
                              }
                              if(!encoder.finish())goto error; return true;
                           }
                        }
                     }
                  }
               }
            error:
               File f; if(f.readStdTry(file)){f.copy(raw.writeMem()); return true;}
            }break;

            case ELM_VIDEO:
            {
               File f; if(f.readStdTry(file)){f.copy(raw.writeMem()); return true;}
            }break;

            case ELM_FILE:
            {
               File f; if(f.readStdTry(file))f.copy(raw.writeMem());
            }return true; // allow replacing with empty file

            case ELM_CODE:
            {
               FileText f; if(f.read(file)){f.getAll(code); return true;}
            }break;

            case ELM_ANIM:
            {
               if(GetExt(file)=="anim") // EE animation
               {
                  XAnimation &xanim=anims.New();
                  if(xanim.anim.load(file)){xanim.name=GetBaseNoExt(file); return true;}
               }else // anim inside mesh
               if(EE.Import(file, null, &skel, anims, null, null, bone_names, all_nodes_as_bones)) // skeleton is needed for anims
                  if(anims.elms()>=1) // at least 1 anim
               {
                  if(anims.elms()>1)
                     if(C TextParam *anim_name=files[0].findParam("name"))
                  {
                     REPA(anims)if(anims[i].name==anim_name.value) // if more than 1 then find the one with matching name
                     {
                        Swap(anims[0], anims[i]); // put to first place
                        break;
                     }
                     anims.setNum(1); // remove all other
                  }
                  XAnimation &anim=anims[0];
                  anim.anim.linear(anim.fps>=LinearAnimFpsLimit);
                  if(files.elms())
                  {
                     if(anim.fps>0) // clip !! do this before looping and speed !!
                     {
                      C TextParam *start_frame=files[0].findParam("start_frame"),
                                  *  end_frame=files[0].findParam(  "end_frame");
                        if(start_frame || end_frame) // clip animation, currently importer will already offset keyframes by 'anim.start', so if we want custom ranges, we need to revert it back
                           anim.anim.clip(start_frame ? start_frame.asFlt()/anim.fps-anim.start : 0,
                                            end_frame ?   end_frame.asFlt()/anim.fps-anim.start : anim.anim.length());
                     }
                     FREPA(files[0].params) // process in order
                     {
                      C TextParam &p=files[0].params[i]; if(p.name=="speedTime") // adjust speed for specified time range !! do this after clipping so the clip isn't affected by speed !!
                        {
                           Vec v=p.asVec(); flt speed=v.x, start=v.y, end=v.z;
                           if(speed)anim.anim.scaleTime(start, end, 1/speed);
                        }
                     }
                     if(C TextParam *p=files[0].findParam("loop")) // set looping !! do this after clipping so the clip isn't affected by looping !!
                     {
                        has_loop=true;
                        anim.anim.loop(p.asBool());
                        files[0].params.removeData(p); // remove this parameter because we've already applied this change to the animation, so when user modifies manually the looping, and then selectes reload, then looping won't be changed
                     }
                     if(C TextParam *p=files[0].findParam("speed")) // adjust speed !! do this after clipping so the clip isn't affected by speed !!
                        if(flt speed=p.asFlt())
                           anim.anim.length(anim.anim.length()/speed, true);
                  }
                  T.file=Edit.FileParams.Encode(files); // 'files' could have changed, so adjust the name so the 'elm.srcFile' is set properly
                  anim.anim.clip(0, anim.anim.length()); // remove any keyframes outside of anim range
                  return true;
               }
            }break;

            case ELM_IMAGE:
            {
               if(files.elms()==6) // special case of cube map
               {
                  has_color=true ; // assume that they have color
                  has_alpha=false; // ignore alpha for cube maps
                  FREPA(files)
                  {
                     ImageEx &image=images(i); if(!ImportImage(image, files[i].name))
                     {
                        image.raw.pos(0); if(image.ImportTry(image.raw))
                        {
                           if(image.cube)image.crop(image, i*image.w()/6, 0, image.w()/6, image.h()); // crop to i-th face for cubes
                        }
                     }
                     Project.TransformImage(image, files[i].params, true);
                  }

                  // check if any of the images were not imported
                  REPA(images)if(!images[i].is())
                  {
                     Image src; raw.pos(0); if(src.ImportTry(raw)) // try extracting from existing data
                     {
                        bool one=(src.aspect()<Avg(1.0, 6.0)); // source is only 1 face, not "6 x face"
                        REPA(images)if(!images[i].is())if(one)src.copyTry(images[i]);else src.crop(images[i], i*src.w()/6, 0, src.w()/6, src.h());
                     }
                     break;
                  }

                  raw.del();

                  // calculate max size
                  int size=0; REPA(images)MAX(size, Max(images[i].w(), images[i].h()));

                  // if actually has some images
                  if(size>0)
                  {
                     Image dest; if(dest.createSoftTry(size*6, size, 1, IMAGE_R8G8B8A8)) // create soft RGBA so we can use simple mem copy
                     {
                        // clear to zero in case some images are not found
                        dest.clear();

                        // insert all images into 6*2D image
                        REPA(images)
                        {
                           Image &src=images[i];
                           if(src.is() && src.copyTry(src, size, size, 1, dest.hwType(), IMAGE_SOFT, 1, FILTER_BEST, true, false, false, false, false)) // copy to the same size and hw type as dest so simple mem copy can be used
                              if(src.lockRead())
                           {
                              // copy non-compressed 2D face to non-compressed 6*2D
                              int byte_pp=ImageTI[dest.hwType()].byte_pp;
                              REPD(y, size)
                              {
                                 Copy(dest.data() + y*dest.pitch() + i*byte_pp*size,
                                      src .data() + y*src .pitch()                 , byte_pp*size);
                              }
                              src.unlock();
                           }
                        }

                        // compress into WEBP raw data
                        dest.ExportWEBP(raw.writeMem(), 1, 1);
                     }
                  }
                  return true;
               }else
               {
                  Str  ext=GetExt(file);
                  bool transforms=(files.elms() && files[0].params.elms()); // if want to apply any transforms
                  if(!transforms && (ext=="jpg" || ext=="jpeg" || ext=="webp"/* || ext=="png"*/)) // images are already in accepted format (even though PNG is compressed, we can achieve much better compression with WEBP)
                  {
                     File f; if(!f.readStdTry(file))break; f.copy(raw.writeMem());
                     if(ext=="jpg" || ext=="jpeg"){Image image; raw.pos(0); if(image.ImportJPG (raw)){has_color=HasColor(image); has_alpha=false          ;}}else // JPG never has any alpha
                     if(ext=="png"               ){Image image; raw.pos(0); if(image.ImportPNG (raw)){has_color=HasColor(image); has_alpha=HasAlpha(image);}}else
                     if(ext=="webp"              ){Image image; raw.pos(0); if(image.ImportWEBP(raw)){has_color=HasColor(image); has_alpha=HasAlpha(image);}}
                     return true;
                  }else // import and export as WEBP
                  {
                     Image image; if(ImportImage(image, file, -1, IMAGE_SOFT, 1, true))
                     {
                        if(files.elms())ProjectEx.TransformImage(image, files[0].params, true);
                        image.ExportWEBP(raw.writeMem(), 1, 1); has_color=HasColor(image); has_alpha=HasAlpha(image); return true;
                     }
                  }
               }
            }break;

            case ELM_MINI_MAP:
            {
               ImageEx image; if(ImportImage(image, file, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))
               {
                  Swap(images.New(), image);
                  return true;
               }
            }break;

            case ELM_MTRL:
            {
               if(GetExt(file)=="mtrl") // EE mtrl
               {
                  MaterialEx &m=mtrls.New(); if(m.mtrl.load(file))
                  {
                     m.createFrom(m.mtrl);
                     m.process(S);
                     return true;
                  }
               }else // mtrl inside mesh
               if(EE.Import(file, null, null, null, SCAST(Memc<XMaterial>, mtrls), null))
                  if(mtrls.elms()>=1) // at least 1 mtrl
               {
                  if(mtrls.elms()>1)
                     if(C TextParam *mtrl_name=files[0].findParam("name"))
                  {
                     REPA(mtrls)if(mtrls[i].name==mtrl_name.value) // if more than 1 then find the one with matching name
                     {
                        Swap(mtrls[0], mtrls[i]); // put to first place
                        break;
                     }
                     mtrls.setNum(1); // remove all other
                  }
                  Str path=GetPath(file); FREPAO(mtrls).process(path);
                  return true;
               }
            }break;

            case ELM_FONT:
            {
               return edit_font.make(font);
            }break;
         }
         return false;
      }
   }

   static void ImportDo(Import &import, ImporterClass &importer, int thread_index)
   {
      ThreadMayUseGPUData();
      import.status=import.import(); // after setting this, do not operate on 'import' anymore as it will be removed
      ThreadFinishedUsingGPUData();
   }
   class ImportElm
   {
      UID  elm_id;
      bool remember_result;
      
      void set(C UID &elm_id, bool remember_result=false) {T.elm_id=elm_id; T.remember_result=remember_result;}
   }

   Memx<Import>                 imports     ; // elements that are being imported right now, Memx due to const mem addr for processing on threads
   Memc<ImportElm>              import_queue; // elements to import
   Map<UID, Edit.RELOAD_RESULT> import_results(Compare); // doesn't need to be thread-safe
   Memc<UID>                    texs; // this is a copy of 'Project.texs' that includes textures currently being imported in 'threads'
   Threads                      threads;
   SyncLock                     lock;
   Menu                         import_menu;
   Memc<Str>                    import_files;
   UID                          import_target=UIDZero;
   MODE                         import_mode=UPDATE;

  ~ImporterClass() {threads.del();} // delete threads before other members

   // get
   bool busy()C {return threads.busy();}

   // manage
   static void ImportFull   (ImporterClass &ic) {Proj.drop(ic.import_files, ic.import_target); ic.import_files.clear();} // import normally
   static void ImportReplace(ImporterClass &ic) {if(ic.import_files.elms())if(Elm *elm=Proj.findElm(ic.import_target)){elm.setSrcFile(ic.import_files[0]); Proj.elmReload(elm.id); Server.setElmShort(elm.id);}}
   static void ImportAnim   (ImporterClass &ic) {ic.importSpecial(ANIM );}
   static void ImportCloth  (ImporterClass &ic) {ic.importSpecial(CLOTH);}
   static void ImportAdd    (ImporterClass &ic) {ic.importSpecial(ADD  );}

   void create()
   {
      {
         Node<MenuElm> n;
         n.New().create("Import as child to object"      , ImportFull   , T).desc("This will perform standard importing, and put the element inside selected object.");
         n.New().create("Import as animation to object"  , ImportAnim   , T).desc("This will import animations only and adjust them to selected object.");
         n.New().create("Import as cloth/armor to object", ImportCloth  , T).desc("This will import mesh as cloth/armor to the selected object,\nmaking mesh use the same skeleton as target object.");
         n.New().create("Import and replace object"      , ImportReplace, T).desc("This will replace current object with data from the file.");
         n.New().create("Import and add to object"       , ImportAdd    , T).desc("This will import mesh and add it to current object.");
         Gui+=import_menu.create(n);
      }
      threads.create(true, Max(1, Cpu.threads()-1)); // leave 1 thread for the main thread
   }

   // operations
   void stop()
   {
      threads     .cancel().wait(); // finish thread processing first before anything else
      imports     .clear();
      import_queue.clear();
      import_files.clear();
   }

   void importNew(C UID &elm_id, C UID &parent_id, C Str &file, MODE mode, ELM_TYPE type, C Str &force_name=S, bool remember_result=false)
   {
      threads.queue(imports.New().set(elm_id, parent_id, file, mode, type, force_name, remember_result), ImportDo, T);
   }
   void importSpecialFile(C Str &file)
   {
      Str ext=GetExt(file);
      if(ExtType(ext)==EXT_MESH)importNew(import_target, import_target, file, import_mode, ELM_OBJ );else
      if(ext=="anim"           )importNew(import_target, import_target, file, import_mode, ELM_ANIM);
   }
   void importSpecialDir(C Str &path)
   {
      for(FileFind ff(path); ff(); )switch(ff.type)
      {
         case FSTD_FILE: importSpecialFile(ff.pathName()); break;
         case FSTD_DIR : importSpecialDir (ff.pathName()); break;
      }
   }
   void importSpecial(MODE mode)
   {
      T.import_mode=mode;
      FREPA(import_files)
      {
         FileInfo fi; if(fi.getSystem(import_files[i]))switch(fi.type)
         {
            case FSTD_FILE: importSpecialFile(import_files[i]); break;
            case FSTD_DIR : importSpecialDir (import_files[i]); break;
         }
      }
   }
   void import(Elm &target, Memc<Str> &files, C Vec2 &screen_pos)
   {
      if(files.elms())
      {
         import_files.clear(); Swap(import_files, files);
         import_target=target.id;
         import_menu.posRU(screen_pos).activate();
      }
   }

   bool includeTex(C UID &tex_id)
   {
      if(tex_id.valid())
      {
         SyncLocker locker(lock);
         return texs.binaryInclude(tex_id, Compare);
      }
      return false;
   }
   void excludeTex(C UID &tex_id)
   {
      if(tex_id.valid())
      {
         SyncLocker locker(lock);
         texs.binaryExclude(tex_id, Compare);
      }
   }
   void clearProj()
   {
      stop();
      import_results.clear();
      {SyncLocker locker(lock); texs.clear();}
   }
   void opened(Project &proj, ElmNode &node)
   {
      if(proj.needUpdate())clearProj();else
      {
         {SyncLocker locker(lock); texs=proj.texs;}
         investigate(node);
      }
   }
   Import* findImport(C UID &elm_id)
   {
      REPA(imports)
      {
         Import &import=imports[i];
         if(import.mode==UPDATE && import.elm_id==elm_id)return &import;
      }
      return null;
   }
   ImportElm* findImportQueue(C UID &elm_id)
   {
      FREPA(import_queue)if(import_queue[i].elm_id==elm_id)return &import_queue[i];
      return null;
   }
   bool inQueue(C UID &elm_id)
   {
      return findImport(elm_id) || findImportQueue(elm_id);
   }
   void cancelImports(C MemPtr<UID> &sorted_elm_ids)
   {
      REPA(imports) // check all 'imports' in case there are multiple imports for the same element
      {
         Import &import=imports[i]; // remember that this 'import' may be processed on secondary thread
         if(import.mode==UPDATE // only this mode operates on 'elm_id' while others treat it as target
         && sorted_elm_ids.binaryHas(import.elm_id, Compare))
         {
            if(threads.cancel(import, ImportDo, T))import.status=0; // if succesfully canceled, then mark as finished, otherwise we need to wait until it will finish on its own
            import.cancel=true;
            if(import.remember_result)*import_results(import.elm_id)=Edit.RELOAD_CANCELED; // if wanted to remember result, then set as canceled
         }
      }
   }
   void cancel(C MemPtr<UID> &elm_ids)
   {
      if(elm_ids.elms())
      {
         Mems<UID> sorted; sorted=elm_ids; sorted.sort(Compare);
         cancelImports(sorted);
         REPA(import_queue)
         {
            ImportElm &import=import_queue[i];
            if(sorted.binaryHas(import.elm_id, Compare))
            {
               if(import.remember_result)*import_results(import.elm_id)=Edit.RELOAD_CANCELED; // if wanted to remember result, then set as canceled
               import_queue.remove(i, true);
            }
         }
      }
   }
   void getResult(C MemPtr<UID> &elms, MemPtr<Edit.IDParam<Edit.RELOAD_RESULT>> results)
   {
      results.setNum(elms.elms()); // pre-alloc
      FREPA(results)
      {
         Edit.IDParam<Edit.RELOAD_RESULT> &result=results[i];
         result.id=elms[i];
         if(Edit.RELOAD_RESULT *r=import_results.find(result.id))result.value=*r;else
         {
          C Elm *elm=Proj.findElm(result.id);
            if(!elm            )result.value=Edit.RELOAD_ELM_NOT_FOUND;else
            if( elm.importing())result.value=Edit.RELOAD_IN_PROGRESS  ;else
                                result.value=Edit.RELOAD_NOT_REQUESTED;
         }
      }
   }
   void clearImportResults(C MemPtr<UID> &elm_ids)
   {
      REPA(elm_ids)import_results.removeKey(elm_ids[i]);
   }
   void forgetResult(C MemPtr<UID> &elm_ids)
   {
      if(elm_ids.elms())
      {
         Mems<UID> sorted; sorted=elm_ids; sorted.sort(Compare);
         REPA(imports)
         {
            Import &import=imports[i]; // remember that this 'import' may be processed on secondary thread
            if(import.mode==UPDATE // only this mode operates on 'elm_id' while others treat it as target
            && import.remember_result // if wanted to remember the result
            && sorted.binaryHas(import.elm_id, Compare))import.remember_result=false;
         }
         REPA(import_queue)
         {
            ImportElm &import=import_queue[i];
            if(import.remember_result
            && sorted.binaryHas(import.elm_id, Compare))import.remember_result=false;
         }
         clearImportResults(sorted);
      }
   }
   void reload(C MemPtr<UID> &elm_ids, bool remember_result)
   {
      Mems<UID> sorted; sorted=elm_ids; sorted.sort(Compare);
                         cancelImports     (sorted);
      if(remember_result)clearImportResults(sorted); // since we'll reload elements, we need to clear any previous results, do this after calling 'cancelImports' which may set RELOAD_CANCELED
      REPA(elm_ids) // add in reversed order because elements last in the queue are imported first
      {
       C UID &elm_id=elm_ids[i];
         if(ImportElm *elm=findImportQueue(elm_id))elm.remember_result|=remember_result;
         else                                      import_queue.New().set(elm_id, remember_result);
      }
   }
   void investigate(Elm &elm)
   {
      if(elm.importing() && !inQueue(elm.id))import_queue.New().set(elm.id);
   }
   void investigate(ElmNode &node)
   {
      if(!Proj.needUpdate())
      {
         SyncLocker locker(lock);
         REPA(node.children) // go from back
         {
            int      child_i=node.children [i];
            ElmNode &child  =Proj.hierarchy[child_i];
            Elm     &elm    =Proj.elms     [child_i];
            if(ImportRemovedElms || !elm.removed())
            {
               investigate(child);
               investigate(elm  );
            }
         }
      }
   }
   void investigate(Memc<UID> &elm_ids)
   {
      if(!Proj.needUpdate())
      {
         SyncLocker locker(lock);
         FREPA(elm_ids)if(Elm *elm=Proj.findElm(elm_ids[i]))if(ImportRemovedElms || elm.finalExists())
         {
            int index=Proj.elms.validIndex(elm); if(InRange(index, Proj.hierarchy))
            {
               investigate(Proj.elms     [index]);
               investigate(Proj.hierarchy[index]);
            }
         }
      }
   }
   void processUpdate(Import &import)
   {
      if(Elm *elm=Proj.findElm(import.elm_id))
      {
         Proj.setListCurSel();
         Proj.closeElm(import.elm_id);
         elm.importing(false);
         switch(elm.type)
         {
            case ELM_MTRL: if(import.mtrls.elms())
            {
               Edit.FileParams fp=import.file; fp.getParam("name").value=import.mtrls[0].name;
               Proj.setMtrl(*elm, import.mtrls[0], fp.encode());
               Server.setElmLong(elm.id);
            }break;

            case ELM_IMAGE:
            {
               elm.setSrcFile(import.file);
               import.raw.pos(0); Proj.imageSet(elm.id, import.raw, import.has_color, import.has_alpha);
            }break;

            case ELM_SOUND:
            case ELM_VIDEO:
            case ELM_FILE :
            {
               elm.setSrcFile(import.file);
               import.raw.pos(0); Proj.fileSet(elm.id, import.raw);
            }break;

            case ELM_CODE:
            {
               elm.setSrcFile(import.file);
               Proj.codeSet(elm.id, import.code);
            }break;

            case ELM_FONT:
            {
               Proj.elmChanging(*elm);
               elm.fontData().newVer();
               Save(import.font, Proj.gamePath(*elm)); Proj.savedGame(*elm);
               Proj.elmChanged(*elm);
               Server.setElmLong(elm.id);
            }break;

            case ELM_ANIM:
            {
               if(import.anims.elms())
               if(ElmAnim *anim_data=elm.animData())
               {
                  Mems<Edit.FileParams> file_params=Edit.FileParams.Decode(import.file);

                  Animation &anim=import.anims[0].anim;
                  anim_data.newVer();
                  anim_data.file_time.getUTC(); // file was changed
                  anim_data.setSrcFile(import.file);
                  if(import.has_loop) // if import has information about looping then use it
                  {
                     anim_data.loop(anim.loop()); anim_data.loop_time.getUTC(); // set from imported animation
                  }else anim.loop(anim_data.loop()); // otherwise keep the old setting
                  anim.linear(anim_data.linear()); // keep old linear

                  if(Elm *skel_elm=Proj.findElm(anim_data.skel_id))if(ElmSkel *skel_data=skel_elm.skelData()) // load target skeleton
                  {
                     anim_data.transform=skel_data.transform; // set desired transform to match target skeleton

                       C Skeleton           *skel=Skeletons(Proj.gamePath(skel_elm.id));
                     EditSkeleton edit_skel; edit_skel.load(Proj.editPath(skel_elm.id));
                     Matrix m=skel_data.transform();

                     if(import.skel.is())
                     {
                        // transform first, in case some methods rely on correct scale
                        import.skel.transform(m);
                               anim.transform(m, import.skel);

                     /* First we have to make skel/anim compatible with 'EditSkeleton', we do this by removing any extra bones from import to match 'EditSkeleton'
                        Animation may contain extra bones not present in the Object's EditSkeleton so just remove them
                        This is important, because if animation skeleton has "A->B->C" bones, but target has "A->C" bones, then in animations "C" bones are stored relative to "B" parent,
                           but they need to be stored relative to "A"
                        This is only for few special cases where Animation FBX files have more bones detected than the base Mesh+Skel FBX */
                        Map<Str8, int> skel_to_node(CompareCI); // map that converts SkelBone name -> EditSkeleton node index
                        bool remove=false; // if we've found any bone that isn't present in EditSkeleton and needs to be removed
                        REPA(import.skel.bones)
                        {
                           int node=edit_skel.findNodeI(import.nodeName(i), import.nodeUID(i)); // use 'bone_names' if available
                           if( node>=0)*skel_to_node(import.skel.bones[i].name)=node;else remove=true; // set only those that were found, others will be removed below
                        }
                        if(remove) // if we need to remove some bones
                        {
                           Skeleton temp=import.skel; REPA(temp.bones)if(!skel_to_node.find(temp.bones[i].name))temp.removeBone(i); // if this bone is not present in 'EditSkeleton' then remove it
                           anim.adjustForSameTransformWithDifferentSkeleton(import.skel, temp); // this will also remove 'anim.bones' not found in skeleton
                           Swap(temp.bones, import.skel.bones);
                        }
                        {
                           // rename from skel bone names to name set from node index to match what we will set in 'temp' skeleton, for example name "leg" gets renamed to "1" if the node index==1
                           REPA(import.skel.bones){SkelBone &bone=import.skel.bones[i]; Set(bone.name, TextInt(*skel_to_node(bone.name)));}
                           REPA(       anim.bones){AnimBone &bone=       anim.bones[i]; Set(bone.name, TextInt(*skel_to_node(bone.name)));}
                           Skeleton temp; edit_skel.set(temp); temp.transform(m);

                           // adjust skeleton pose
                           anim.adjustForSameSkeletonWithDifferentPose(import.skel, temp);
                           Swap(temp.bones, import.skel.bones);
                        }
                        {
                           Mems<Mems<IndexWeight>> weights; edit_skel.set(weights, import.skel, *skel, EditSkeleton.BONE_NAME_IS_NODE_INDEX);
                           int root=edit_skel.root;
                           if(anim_data.flag&ElmAnim.ROOT_FROM_BODY) // if want from body, then override 'root' bone index, and set custom
                           {
                              int bone=Max(0, skel.findBoneI(BONE_SPINE));
                              if(InRange(bone, skel.bones))root=edit_skel.boneToNode(skel.bones[bone].name);
                           }
                           anim.adjustForSameTransformWithDifferentSkeleton(import.skel, *skel, root, weights, anim_data.rootFlags()|(edit_skel.rootZero() ? 0 : ROOT_BONE_POSITION|ROOT_START_IDENTITY)); // don't set ROOT_BONE_POSITION for 'rootZero' because we actually want to force zero bone position, this is needed in case original skeleton root bone pos is zero, and skeleton is transformed with custom position offset, in which case 'import.skel' root bone pos will not be zero and 'edit_skel' (in original transform) root bone pos is zero
                           anim_data.setRoot(anim);
                        }
                     }else
                     {
                        anim.setBoneNameTypeIndexesFromSkeleton(*skel);
                        anim.transform(m, *skel);
                     }

                     // optimize (after transform because scale affects position key removal, after 'adjustForSameTransformWithDifferentSkeleton' so it can operate on highest precision and to cleanup keyframes generated by it)
                     {
                        flt angle_eps=EPS_ANIM_ANGLE, pos_eps=EPS_ANIM_POS, scale_eps=EPS_ANIM_SCALE;
                        if(file_params.elms())if(C TextParam *optimize=file_params[0].findParam("optimize")){flt o=optimize.asFlt(); angle_eps*=o; pos_eps*=o; scale_eps*=o;}
                        anim.optimize(angle_eps, pos_eps, scale_eps);
                     }
                     // mirror
                     {
                        if(file_params.elms())if(C TextParam *mirror=file_params[0].findParam("mirror"))if(mirror.asBool1())anim.mirror(*skel);
                     }
                  }

                  // set events, save and send
                  Str path=Proj.gamePath(elm.id);
                  if(!anim.events.elms()) // if new animation doesn't have events, then check old anim for them
                  {
                     Animation old; if(old.load(path))if(old.events.elms()) // if old had events
                     {
                        anim.events=old.events; // copy them
                        if(old.length()>EPS && anim.length()>EPS)if(flt mul=anim.length()/old.length())REPAO(anim.events).time*=mul; // adjust time
                     }
                  }
                  Proj.elmChanging(*elm);
                  Save(anim, path); Proj.savedGame(*elm, path);
                  Proj.elmChanged(*elm);
                  Server.setElmLong(elm.id);
               }
            }break;

            case ELM_OBJ: if(ElmObj *obj_data=elm.objData())
            {
               obj_data.newVer();
               obj_data.setSrcFile(import.file);
               Skeleton &new_skel=import.skel;

               if(import.mesh.is())Proj.getObjMeshElm(elm.id, false, false); // need to have mesh     element to insert mesh
               if(   new_skel.is())Proj.getObjSkelElm(elm.id, false, false); // need to have skeleton element to insert skeleton

               // update
               if(Elm *mesh_elm=Proj.findElm(obj_data.mesh_id))
                  if(ElmMesh *mesh_data=mesh_elm.meshData())
               {
                  Edit.FileParams fp=import.file; // !! watch out because this has param 'name' changed throughout the codes !!

                              Proj.elmChanging(*     elm);
                  if(mesh_elm)Proj.elmChanging(*mesh_elm);

                  // update skeleton
                  Elm *skel_elm=null;
                  if(new_skel.is()) // only if there's a new skeleton, otherwise keep the current
                     if(skel_elm=Proj.findElm(mesh_data.skel_id))
                        if(ElmSkel *skel_data=skel_elm.skelData())
                  {
                     if(skel_elm)Proj.elmChanging(*skel_elm);

                     skel_data.newVer();
                     skel_data.setSrcFile(import.file);
                     skel_data.file_time.getUTC();

                     Matrix m=skel_data.transform();

                     Str          skel_game_path=Proj.gamePath(skel_elm.id), skel_edit_path=Proj.editPath(skel_elm.id);
                     Skeleton    *old_skel=Skeletons(skel_game_path);
                     EditSkeleton old_edit_skel, new_edit_skel; old_edit_skel.load(skel_edit_path); new_edit_skel.create(new_skel, import.bone_names); // !! create before transform is applied !!
                     new_skel.transform(m); // !! transform after creating 'EditSkeleton' !!

                     // keep Skeleton params and set bone weights from new to old
                     Mems<Mems<IndexWeight>> bone_weights; bone_weights.setNum(new_skel.bones.elms());
                     {
                        // slots
                        new_skel.slots=old_skel.slots; REPA(new_skel.slots)
                        {
                           SkelSlot &slot=new_skel.slots[i];

                           slot.bone=0xFF; for(int old_bone_i=slot.bone; C SkelBone *old_bone=old_skel.bones.addr(old_bone_i); old_bone_i=old_skel.boneParent(old_bone_i)) // iterate bone and its parents
                              if(C EditSkeleton.Bone *old_edit_bone=old_edit_skel.findBone(old_bone.name))
                                 FREPA(*old_edit_bone) // iterate all Bone->Node links
                           {
                                                 int  old_edit_node_i=(*old_edit_bone)[i].index;
                              if(C EditSkeleton.Node *old_edit_node  =  old_edit_skel.nodes.addr(old_edit_node_i))
                              {
                                 int new_node_bone =new_edit_skel.findNodeI(old_edit_node.name, old_edit_skel.nodeUID(old_edit_node_i)); // 'new_edit_skel' nodes have the same order/indexes as bones
                                 if( new_node_bone>=0){slot.bone=new_node_bone; goto found;}
                              }
                           }
                        found:;

                           slot.bone1=0xFF; for(int old_bone_i=slot.bone1; C SkelBone *old_bone=old_skel.bones.addr(old_bone_i); old_bone_i=old_skel.boneParent(old_bone_i)) // iterate bone and its parents
                              if(C EditSkeleton.Bone *old_edit_bone=old_edit_skel.findBone(old_bone.name))
                                 FREPA(*old_edit_bone) // iterate all Bone->Node links
                           {
                                                 int  old_edit_node_i=(*old_edit_bone)[i].index;
                              if(C EditSkeleton.Node *old_edit_node  =  old_edit_skel.nodes.addr(old_edit_node_i))
                              {
                                 int new_node_bone =new_edit_skel.findNodeI(old_edit_node.name, old_edit_skel.nodeUID(old_edit_node_i)); // 'new_edit_skel' nodes have the same order/indexes as bones
                                 if( new_node_bone>=0){slot.bone1=new_node_bone; goto found1;}
                              }
                           }
                        found1:;
                        }
                        // bones
                        REPA(new_skel.bones)
                           if(InRange(i, new_edit_skel.nodes)) // 'new_edit_skel' nodes have the same order/indexes as bones
                        {
                           int old_node_i =old_edit_skel.findNodeI(new_edit_skel.nodes[i].name, new_edit_skel.nodeUID(i));
                           if( old_node_i>=0)
                           {
                              int best_old_skel_bone_i=-1; flt weight; REPAD(b, old_edit_skel.bones)
                              {
                               C EditSkeleton.Bone &bone=old_edit_skel.bones[b]; REPAD(bw, bone)
                                 {
                                  C IndexWeight &iw=bone[bw]; if(iw.index==old_node_i)
                                    {
                                       int old_skel_bone_i=old_skel.findBoneI(bone.name); if(old_skel_bone_i>=0)
                                       {
                                          bone_weights[i].New().set(old_skel_bone_i, iw.weight);
                                          if(best_old_skel_bone_i<0 || iw.weight>weight){best_old_skel_bone_i=old_skel_bone_i; weight=iw.weight;}
                                       }
                                    }
                                 }
                              }
                              if(best_old_skel_bone_i>=0) // copy 'SkelBone' params from an old bone with highest weight
                              {
                               C SkelBone & src_bone=old_skel.bones[best_old_skel_bone_i];
                                 SkelBone &dest_bone=new_skel.bones[i];
                                 dest_bone.width=src_bone.width;
                               //dest_bone.frac =src_bone.frac ;
                                 dest_bone.flag =src_bone.flag ;
                              }
                           }
                        }
                     }
                     Proj.adjustAnimations(skel_elm.id, old_edit_skel, *old_skel, new_skel, bone_weights); // !! convert animations before saving skeleton which modifies 'old_skel' which is a pointer to cache element !!

                     Save(new_edit_skel, skel_edit_path);
                     Save(new_skel     , skel_game_path); Proj.savedGame(*skel_elm, skel_game_path);
                     Server.setElmFull(mesh_data.skel_id);

                     // process animations (they will be available only during first import, and not for "reload", so we don't need to do any fancy conversions)
                     FREPA(import.anims)
                     {
                        XAnimation &xanim=import.anims[i];
                        Elm        & anim=Proj.Project.newElm(xanim.name.is() ? xanim.name : "Animation", elm.id, ELM_ANIM);
                        if(ElmAnim * anim_data=anim.animData())
                        {
                           fp.getParam("name").value=xanim.name;
                           anim_data.newData();
                           anim_data.skel_id  =skel_elm .id;
                           anim_data.transform=skel_data.transform;
                           anim_data.loop     (xanim.anim.loop()).linear(xanim.anim.linear());
                           anim_data.src_file =fp.encode();
                        }
                        xanim.anim.transform(m, new_skel);
                        xanim.anim.optimize(); // optimize after transform
                        Save(xanim.anim, Proj.gamePath(anim)); Proj.savedGame(anim);
                        Server.setElmFull(anim.id);
                     }
                  }

                  // setup materials before mesh
                  Memt<UID> mtrls;
                  FREPA(import.mtrls)
                  {
                     Elm *found_mtrl=null; int match=0;
                     Import.MaterialEx &src=import.mtrls[i];
                     fp.getParam("name").value=src.name; Str mtrl_src_file=fp.encode();
                     REPA(Proj.elms) // find that material
                     {
                        Elm &mtrl=Proj.elms[i];
                        if(ElmMaterial *mtrl_data=mtrl.mtrlData())if(mtrl.finalExists())
                        {
                           int m=0;
                           if(mtrl.parent_id==elm.id) // material is inside this object
                           {
                              if(EqualPath(mtrl_data.src_file,          mtrl_src_file))m=3;else // full path the same - perfect match
                              if(  GetBase(mtrl_data.src_file)==GetBase(mtrl_src_file))m=2;     // base name the same
                           }else
                           {
                              if(EqualPath(mtrl_data.src_file,          mtrl_src_file))m=1; // full path the same, but not inside the object, perhaps this material is used for something else, prefer materials inside the object
                           }
                           if(m>match){match=m; found_mtrl=&mtrl;}
                        }
                     }
                     if(!found_mtrl) // if haven't found then create a new one
                     {
                        found_mtrl=&Proj.newMtrl(src, elm.id, mtrl_src_file);
                        Server.setElmFull(found_mtrl.id);
                     }
                     mtrls.add(found_mtrl.id);
                  }

                  // update mesh
                  mesh_data.newVer();
                  mesh_data.setSrcFile(import.file);
                  mesh_data.file_time.getUTC();
                  int pmi=0; FREPD(l, import.mesh.lods()) // set materials
                  {
                     MeshLod &lod=import.mesh.lod(l); FREPA(lod)
                     {
                        if(InRange(pmi, import.part_mtrl_index))
                        {
                           int mtrl_index=import.part_mtrl_index[pmi];
                           if(InRange(mtrl_index, mtrls))lod.parts[i].material(Proj.gamePath(mtrls[mtrl_index]));
                        }
                        pmi++;
                     }
                  }
                  Str  edit_path=Proj.editPath(mesh_elm.id);
                  Mesh old_mesh; if(Load(old_mesh, edit_path, Proj.game_path))KeepParams(old_mesh, import.mesh);
                  Save(import.mesh, edit_path, Proj.game_path); // save

                  // game mesh
                  Skeleton *body_skel; Proj.getMeshSkels(mesh_data, null, &body_skel);
                  Mesh game_mesh; EditToGameMesh(import.mesh, game_mesh, body_skel, Proj.getEnum(mesh_data.draw_group_id), &mesh_data.transform());
                  Save(game_mesh, Proj.gamePath(mesh_elm.id)); Proj.savedGame(*mesh_elm); // save
                  mesh_data.from(game_mesh);
                  Server.setElmFull(obj_data.mesh_id);

                  // notify about change
                              Proj.elmChanged(*     elm);
                  if(mesh_elm)Proj.elmChanged(*mesh_elm);
                  if(skel_elm)Proj.elmChanged(*skel_elm);

                  // send to server
                  Server.setElmShort(elm.id);
               }
            }break;

            case ELM_MINI_MAP:
            {
               if(import.images.elms()==1)
                  if(ElmMiniMap *mini_map_data=elm.miniMapData())
                     if(MiniMapVer *ver=Proj.miniMapVerGet(elm.id))
                        if(ver.images.elms())
                           if(int image_size=ver.settings.image_size)
               {
                  Proj.elmChanging(*elm);
                  mini_map_data.newVer();
                  mini_map_data.setSrcFile(import.file);
                  RectI images=ver.images.last(); REPA(ver.images)images|=ver.images[i];
                  VecI2 images_size=image_size*(images.size()+1);
                  Image &all=import.images[0], single, temp;
                  if(all.copyTry(all, images_size.x, images_size.y, 1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1)
                  && single.createSoftTry(image_size, image_size, 1, IMAGE_R8G8B8A8))
                  {
                     ver.changed=true;
                     ver.time.getUTC(); // update time of mini map
                     Server.setMiniMapSettings(elm.id, ver.settings, ver.time); // send updated mini map settings first, as images will be received only if their timestamp matches the settings
                     REPA(ver.images)
                     {
                      C VecI2 &image_pos=ver.images[i];
                        int ox=(image_pos.x-images.min.x            )*image_size,
                            oy=(            images.max.y-image_pos.y)*image_size;
                        REPD(y, single.h())
                        REPD(x, single.w())single.pixel(x, y, all.pixel(x+ox, y+oy));
                        if(single.copyTry(temp, -1, -1, -1, IMAGE_BC1, IMAGE_2D, 1))
                        {
                           temp.save(Proj.gamePath(elm.id).tailSlash(true)+image_pos);
                           Synchronizer.setMiniMapImage(elm.id, image_pos);
                        }
                     }
                     Server.setElmShort(elm.id);
                     Proj.elmChanged(*elm);
                  }
               }
            }break;
         }
         Proj.setList(); // refresh list because 'importing' affects elm color, and ELM_OBJ could have created new elements
      }
   }
   void processCloth(Import &import)
   {
      if(import.mesh.is())
      if(Elm     *obj_elm=Proj.findElm(import.elm_id, ELM_OBJ))
      if(ElmObj  *obj_data=obj_elm.objData())
      if(Elm     *body_elm=Proj.findElm(obj_data.mesh_id))
      if(ElmMesh *body_mesh_data=body_elm.meshData())
      {
         if(Elm *parent=Proj.findElm(import.parent_id))parent.opened(true);
         Elm &cloth=Proj.Project.newElm(GetBaseNoExt(import.file), import.parent_id, ELM_OBJ);
         if(Elm *cloth_mesh=Proj.getObjMeshElm(cloth.id))
            if(ElmMesh *mesh_data=cloth_mesh.meshData())
         {
            mesh_data.newData();
            mesh_data.body_id  =      obj_data.mesh_id  ;
            mesh_data.transform=body_mesh_data.transform;
         }
         import.elm_id=cloth.id; // change id to cloth because we want to set it
         processUpdate(import); // this will handle setting elm list and sending to server
      }
   }
   void processAnim(Import &import)
   {
    //if(import.skel.is()) don't check this because we can import also EE.Animation ('anim' file format) which doesn't contain a skeleton
      if(import.anims.elms())
      if(  Elm      *obj_elm=Proj.findElm(import.elm_id, ELM_OBJ))
      if(  ElmObj   *obj_data=obj_elm.objData())
      if(  Elm      *mesh_elm=Proj.findElm(obj_data.mesh_id))
      if(  ElmMesh  *mesh_data=mesh_elm.meshData())
      if(  Elm      *skel_elm=Proj.findElm(mesh_data.skel_id))
      if(  ElmSkel  *skel_data=skel_elm.skelData())
      if(C Skeleton *skel=Skeletons(Proj.gamePath(skel_elm.id)))
      {
         Edit.FileParams fp=import.file;
         EditSkeleton edit_skel; edit_skel.load(Proj.editPath(skel_elm.id)); // 'edit_skel' is always in identity matrix
         Matrix m=skel_data.transform();

         if(import.skel.is())
         {
            // transform first, in case some methods rely on correct scale
                         import.skel.transform(m);
            REPAO(import.anims).anim.transform(m, import.skel);

            Map<Str8, int> skel_to_node(CompareCI); // map that converts SkelBone name -> EditSkeleton node index
            bool remove=false; // if we've found any bone that isn't present in EditSkeleton and needs to be removed
            REPA(import.skel.bones)
            {
               int node=edit_skel.findNodeI(import.nodeName(i), import.nodeUID(i)); // use 'bone_names' if available
               if( node>=0)*skel_to_node(import.skel.bones[i].name)=node;else remove=true; // set only those that were found, others will be removed below
            }
            if(remove) // if we need to remove some bones
            {
               Skeleton temp=import.skel; REPA(temp.bones)if(!skel_to_node.find(temp.bones[i].name))temp.removeBone(i); // if this bone is not present in 'EditSkeleton' then remove it
               REPAO(import.anims).anim.adjustForSameTransformWithDifferentSkeleton(import.skel, temp); // this will also remove 'anim.bones' not found in skeleton
               Swap(temp.bones, import.skel.bones);
            }
            {
               // rename from skel bone names to name set from node index to match what we will set in 'temp' skeleton, for example name "leg" gets renamed to "1" if the node index==1
                                                                        REPA(import.skel.bones){SkelBone &bone=import.skel.bones[i]; Set(bone.name, TextInt(*skel_to_node(bone.name)));}
               REPA(import.anims){Animation &anim=import.anims[i].anim; REPA(       anim.bones){AnimBone &bone=       anim.bones[i]; Set(bone.name, TextInt(*skel_to_node(bone.name)));}}
               Skeleton temp; edit_skel.set(temp); temp.transform(m);

               // adjust skeleton pose
               REPAO(import.anims).anim.adjustForSameSkeletonWithDifferentPose(import.skel, temp);
               Swap(temp.bones, import.skel.bones);
            }
            {
               Mems<Mems<IndexWeight>> weights; edit_skel.set(weights, import.skel, *skel, EditSkeleton.BONE_NAME_IS_NODE_INDEX);
               REPAO(import.anims).anim.adjustForSameTransformWithDifferentSkeleton(import.skel, *skel, edit_skel.root, weights, (edit_skel.rootZero() ? 0 : ROOT_BONE_POSITION|ROOT_START_IDENTITY));
            }
         }else
         {
            REPA(import.anims)
            {
               Animation &anim=import.anims[i].anim;
               anim.setBoneNameTypeIndexesFromSkeleton(*skel);
               anim.transform(m, *skel);
            }
         }

         // optimize (after transform because scale affects position key removal, after 'adjustForSameTransformWithDifferentSkeleton' so it can operate on highest precision and to cleanup keyframes generated by it)
         {
            REPAO(import.anims).anim.optimize();
         }

         Proj.setListCurSel();
         FREPA(import.anims)
         {
            XAnimation &xanim=import.anims[i];

            // adjust name
            if(!import.force_name.is() && import.anims.elms()==1 && (xanim.name=="Take 01" || xanim.name=="Take 001" || xanim.name=="Default Take" || xanim.name=="C4D Animation Take" || xanim.name=="mixamo.com" || xanim.name=="Unreal Take"))
            {
               import.force_name=Replace(GetBaseNoExt(import.file), '_', ' ');
               REPA(import.force_name)if(import.force_name[i]=='@') // check for unity style file names (one anim per file, name "take 01" or "take 001"), file name "Forest Wolf@Bite Attack.FBX"
               {
                  import.force_name.remove(0, i+1);
                  break;
               }
            }

            // create element
            Elm        &anim=Proj.Project.newElm(import.force_name.is() ? import.force_name : xanim.name.is() ? xanim.name : GetBaseNoExt(Edit.FileParams(import.file).name), import.parent_id, ELM_ANIM);
            if(ElmAnim *anim_data=anim.animData())
            {
               fp.getParam("name").value=xanim.name;
               anim_data.newData();
               anim_data.skel_id  =skel_elm.id;
               anim_data.transform=skel_data.transform;
               anim_data.loop     (xanim.anim.loop()).linear(xanim.anim.linear());
               anim_data.src_file =fp.encode();
            }

            // save and send
            Save(xanim.anim, Proj.gamePath(anim.id)); Proj.savedGame(anim);
            Server.setElmFull(anim.id);
         }
         Proj.setList();
      }
   }
   void processAdd(Import &import)
   {
      Proj.closeElm(import.elm_id);
      if(Elm *elm=Proj.findElm(import.elm_id))switch(elm.type)
      {
         case ELM_OBJ:
         {
            if(ElmObj *obj_data=elm.objData())
            {
               Proj.setListCurSel();
               obj_data.newVer();
               obj_data.setSrcFile(Edit.FileParams.Merge(obj_data.src_file, import.file)); // add to file list

               if(import.mesh.is())Proj.getObjMeshElm(elm.id, false, false); // need to have mesh     element to insert mesh
               if(import.skel.is())Proj.getObjSkelElm(elm.id, false, false); // need to have skeleton element to insert skeleton

               // update
               if(Elm *mesh_elm=Proj.findElm(obj_data.mesh_id))
                  if(ElmMesh *mesh_data=mesh_elm.meshData())
               {
                  // update skeleton
                  Elm *skel_elm=null; // currently this is not done

                              Proj.elmChanging(*     elm);
                  if(mesh_elm)Proj.elmChanging(*mesh_elm);
                  if(skel_elm)Proj.elmChanging(*skel_elm);

                  // first setup materials
                  Edit.FileParams fp=import.file;
                  Memt<UID> mtrls;
                  FREPA(import.mtrls)
                  {
                     Elm *found_mtrl=null; int match=0;
                     Import.MaterialEx &src=import.mtrls[i];
                     fp.getParam("name").value=src.name; Str mtrl_src_file=fp.encode();
                     REPA(Proj.elms) // find that material
                     {
                        Elm &mtrl=Proj.elms[i];
                        if(ElmMaterial *mtrl_data=mtrl.mtrlData())if(mtrl.finalExists())
                        {
                           int m=0;
                           if(mtrl.parent_id==elm.id) // material is inside this object
                           {
                              if(EqualPath(mtrl_data.src_file,          mtrl_src_file))m=3;else // full path the same - perfect match
                              if(  GetBase(mtrl_data.src_file)==GetBase(mtrl_src_file))m=2;     // base name the same
                           }else
                           {
                              if(EqualPath(mtrl_data.src_file,          mtrl_src_file))m=1; // full path the same, but not inside the object, perhaps this material is used for something else, prefer materials inside the object
                           }
                           if(m>match){match=m; found_mtrl=&mtrl;}
                        }
                     }
                     if(!found_mtrl) // if haven't found then create a new one
                     {
                        found_mtrl=&Proj.newMtrl(src, elm.id, mtrl_src_file);
                        Server.setElmFull(found_mtrl.id);
                     }
                     mtrls.add(found_mtrl.id);
                  }

                  // update mesh
                  mesh_data.newVer();
                  mesh_data.setSrcFile(Edit.FileParams.Merge(mesh_data.src_file, import.file)); // add to file list
                  mesh_data.file_time.getUTC();
                  int pmi=0; FREPD(l, import.mesh.lods()) // set materials
                  {
                     MeshLod &lod=import.mesh.lod(l); FREPA(lod)
                     {
                        if(InRange(pmi, import.part_mtrl_index))
                        {
                           int mtrl_index=import.part_mtrl_index[pmi];
                           if(InRange(mtrl_index, mtrls))lod.parts[i].material(Proj.gamePath(mtrls[mtrl_index]));
                        }
                        pmi++;
                     }
                  }
                  Str  edit_path=Proj.editPath(mesh_elm.id);
                  Mesh old_mesh; if(Load(old_mesh, edit_path, Proj.game_path))
                  {
                     old_mesh.add(import.mesh);
                     Swap(old_mesh, import.mesh);
                  }
                  Save(import.mesh, edit_path, Proj.game_path); // save

                  // game mesh
                  Skeleton *body_skel; Proj.getMeshSkels(mesh_data, null, &body_skel);
                  Mesh game_mesh; EditToGameMesh(import.mesh, game_mesh, body_skel, Proj.getEnum(mesh_data.draw_group_id), &mesh_data.transform());
                  Save(game_mesh, Proj.gamePath(mesh_elm.id)); Proj.savedGame(*mesh_elm); // save
                  mesh_data.from(game_mesh);

                  // notify about change
                              Proj.elmChanged(*     elm);
                  if(mesh_elm)Proj.elmChanged(*mesh_elm);
                  if(skel_elm)Proj.elmChanged(*skel_elm);

                  // send to server
                  Server.setElmFull(obj_data.mesh_id); Server.setElmFull(mesh_data.skel_id); Server.setElmShort(elm.id);
               }
               Proj.setList(); // refresh because object may have added new elements
            }
         }break;
      }
   }

   // update
   void update()
   {
      // process imported
      REPA(imports)
      {
         Import &import=imports[i]; if(import.status>=0) // finished importing
         {
            if(!import.cancel)
            {
               if(import.remember_result)*import_results(import.elm_id)=(import.status ? Edit.RELOAD_SUCCESS : Edit.RELOAD_FAILED);
               if(import.status)switch(import.mode) // ok
               {
                  case UPDATE: processUpdate(import); break;
                  case ANIM  : processAnim  (import); break;
                  case CLOTH : processCloth (import); break;
                  case ADD   : processAdd   (import); break;
               }
            }
            imports.removeValid(i);
         }
      }

      // process queue
      for(; import_queue.elms() && imports.elms()<Max(threads.threads()*2, 15); ) // queue 15 elements per frame, so import threads can process before waiting for this code to execute on the next frame
      {
       C ImportElm &import=import_queue.last();
         if(Elm *elm=Proj.findElm(import.elm_id))if(ImportRemovedElms || elm.finalExists())
            importNew(elm.id, elm.parent_id, elm.srcFile(), UPDATE, elm.type, S, import.remember_result);
         import_queue.removeLast();
      }
   }
}
ImporterClass Importer;
/******************************************************************************/
