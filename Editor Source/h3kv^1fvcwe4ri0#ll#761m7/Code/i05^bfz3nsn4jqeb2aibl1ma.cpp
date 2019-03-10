/******************************************************************************/
class ExportWindow : WindowIO
{
   UID elm_id=UIDZero;
   Str ext;

   static void Save(C Str &name, ptr) {Export.save(name);}
          void save(C Str &name)
   {
      bool ok=false;
      if(Elm *elm=Proj.findElm(elm_id))
      {
         Proj.flushElm(elm.id);
         switch(elm.type)
         {
            case ELM_ANIM:
            {
               Animation anim; if(anim.load(Proj.gamePath(elm_id)))
               {
                  if(ElmAnim *anim_data=elm.animData())
                  {
                     Skeleton skel; if(anim_data.skel_id.valid())skel.load(Proj.gamePath(anim_data.skel_id));
                     anim.transform(~anim_data.transform(), skel); // restore to original transformation
                  }
                  ok=anim.save(name);
               }
            }break;

            case ELM_OBJ:
            {
               if(C Elm *mesh_elm=Proj.findElm(elm.objData().mesh_id, ELM_MESH))
               {
                  Mesh mesh; if(Load(mesh, Proj.editPath(mesh_elm.id), Proj.game_path))
                  {
                     ok=ExportOBJ(name, mesh);
                  }
               }else Gui.msgBox(S, "Object doesn't have any Mesh");
            }break;
            
            case ELM_SKEL:
            {
               Skeleton skel; if(skel.load(Proj.gamePath(elm_id)))
               {
                  if(ElmSkel *skel_data=elm.skelData())skel.transform(~skel_data.transform()); // restore to original transformation
                  ok=skel.save(name);
               }
            }break;
            
            case ELM_IMAGE:
            case ELM_ICON:
            {
               Str src=Proj.basePath(*elm);
               if(GetExt(name)==ext)ok=FCopy(src, name);else
               {
                  Image image; if(image.ImportTry(src))ok=image.Export(name);
               }
            }break;

            case ELM_PANEL_IMAGE:
            {
               PanelImage panel; if(panel.load(Proj.gamePath(elm_id)))ok=panel.image.Export(name);
            }break;

            case ELM_CODE:
            {
               Str code; if(Proj.codeGet(elm_id, code))
               {
                  FileText f; if(ok=f.write(name))f.putText(code);
               }
            }break;

            case ELM_IMAGE_ATLAS:
            case ELM_PHYS_MTRL:
            case ELM_FONT:
            case ELM_SOUND:
            case ELM_VIDEO:
            case ELM_FILE:
            {
               ok=FCopy(Proj.gamePath(elm_id), name);
            }break;
            
            case ELM_MINI_MAP:
            {
               if(ElmMiniMap *mini_map_data=elm.miniMapData())
                  if(MiniMapVer *ver=Proj.miniMapVerGet(elm_id))
                     if(ver.images.elms())
                     if(int image_size=ver.settings.image_size)
               {
                  Game.MiniMap map; if(map.load(Proj.gamePath(elm_id)))
                  {
                     RectI images=ver.images.last(); REPA(ver.images)images|=ver.images[i];
                     Image image, temp; if(image.createSoftTry(image_size*(images.w()+1), image_size*(images.h()+1), 1, IMAGE_R8G8B8A8))
                     {
                        image.clear();
                        for(int y=images.min.y; y<=images.max.y; y++)
                        for(int x=images.min.x; x<=images.max.x; x++)
                        {
                           C Image &src=map(VecI2(x, y));
                           if(src.is() && src.copyTry(temp, image_size, image_size, 1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))
                           {
                              int ox=(x-images.min.x  )*image_size,
                                  oy=(  images.max.y-y)*image_size;
                              REPD(y, temp.h())
                              REPD(x, temp.w())image.pixel(x+ox, y+oy, temp.pixel(x, y));
                           }
                           map.clear();
                        }
                        ok=image.Export(name);
                     }
                  }
               }
            }break;
         }
      }
      if(ok)Explore(name, true);
   }

   void activate(C UID &elm_id)
   {
      hide();
      T.elm_id=elm_id;
      if(Elm *elm=Proj.findElm(elm_id))
      {
         ext.clear(); Str supp_ext;
         switch(elm.type)
         {
            default: return;

            case ELM_IMAGE:
            {
              .File f; if(f.readTry(Proj.editPath(elm_id))) // set main extension
               {
                  Image temp;
                //if(!ext.is()){f.pos(0); if(temp.ImportWEBP(f))ext="webp";} ignore WEBP because it's not popular yet
                  if(!ext.is()){f.pos(0); if(temp.ImportPNG (f))ext="png" ;}
                  if(!ext.is()){f.pos(0); if(temp.ImportJPG (f))ext="jpg" ;}
               }
               supp_ext=ext+"|bmp|png|jpg|tga|tif|webp|ico|icns|img"; // add secondary extensions
            }break;

            case ELM_PANEL_IMAGE:
            case ELM_MINI_MAP   : supp_ext="bmp|png|jpg|tga|tif|webp|img"; break;

            case ELM_OBJ        : ext="obj"; break;
            case ELM_ANIM       : ext="anim"; break;
            case ELM_FONT       : ext="font"; break;
            case ELM_IMAGE_ATLAS: ext="atlas"; break;
            case ELM_SKEL       : ext="skel"; break;
            case ELM_PHYS_MTRL  : ext="physmtrl"; break;
            case ELM_ICON       : ext="img"; supp_ext="bmp|png|jpg|tga|tif|webp|img"; break;
            case ELM_CODE       : supp_ext="txt|c|cpp|h|cs|cc|cxx|m|mm"; break;

            case ELM_SOUND: {SoundHeader sound; sound.load(Proj.gamePath(elm_id)); ext=CaseDown(sound.codecName());} break;
            case ELM_VIDEO:
            {
               Video video; if(video.create(Proj.gamePath(elm_id)))switch(video.codec())
               {
                  case VIDEO_THEORA: ext="ogv" ; break;
                  case VIDEO_VP9   : ext="webm"; break;
               }
            }break;

            case ELM_FILE: ext=GetExt(elm.srcFile()); break;
         }
         if(!supp_ext.is())supp_ext=ext;
         if(!is())super.create().io(null, Save);
         super.ext(supp_ext).save().name(elm.name).textline.selectAll();
      }
   }
}
ExportWindow Export;
/******************************************************************************/
