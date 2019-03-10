/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
ObjClassEditor ObjClassEdit;
/******************************************************************************/

/******************************************************************************/
      const flt ParamEditor::ParamWindow::elm_h=0.042f;
   NameDesc ParamEditor::path_mode[]=
   {
      {MLTC(u"Create"     , PL, u"Utwórz"             ), u"Object will be included in Path Mesh generation."},
      {MLTC(u"Ignore"     , PL, u"Ignoruj"            ), u"Object will not be included in Path Mesh generation."},
      {MLTC(u"Fully Block", PL, u"Całkowicie Zablokuj"), u"Object will fully block any passage at its location."},
   };
   NameDesc ParamEditor::etype_mode[]=
   {
      {u"Mesh"       , u"Object will be drawn using its mesh."},
      {u"Point Light", u"Object will be drawn as point light."},
      {u"Cone Light" , u"Object will be drawn as cone light."},
      {u"Particles"  , u"Object will be drawn as particles."},
   };
/******************************************************************************/
      ParamEditor::RenameWindow::RenameWindow(ParamEditor &pe) : pe(pe) {}
      void ParamEditor::RenameWindow::create()
      {
         Gui+=::EE::Window::create(Rect_C(0, 0, 1, 0.14f), "Rename").hide(); button[2].show();
         T  +=dest .create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01f));
      }
      void ParamEditor::RenameWindow::rename(C Str &src, PARAM_TYPE type, C UID &id)
      {
         data.name=src ;
         data.type=type;
         T   .id  =id  ;
         setTitle(S+"Rename \""+src+"\" parameter");
         activate();
         dest.set(src).selectAll().activate();
      }
      bool ParamEditor::RenameWindow::Rename(ParamEditor &pe, EditObject &obj, C RenameWindow &rename)
      {
         bool changed=false; REPA(obj)
         {
            EditParam &param=obj[i]; if(ParamCompatible(param, rename.data) && !param.removed){param.setName(rename.dest()); changed=true;}
         }
         return changed;
      }
      void ParamEditor::RenameWindow::update(C GuiPC &gpc)
{
         ::EE::ClosableWindow::update(gpc);

         if(Gui.window()==this && Kb.k(KB_ENTER))
         {
            if(pe.world)
            {
               REPA(Selection)
               {
                  Obj &obj=Selection[i];
                  REPA(obj.params){EditParam &param=obj.params[i]; if(ParamCompatible(param, data) && !param.removed){obj.setUndo(); param.setName(dest()); obj.setChanged();}}
               }
            }else
            {
               pe.setUndo(this);
               REPA(*pe.p){EditParam &param=(*pe.p)[i]; if(param.id==id && !param.removed)param.setName(dest());}
               pe.setChanged(false);
            }
            pe.multiFunc(Rename, T);
            pe.toGui();
            hide();
         }
      }
         ::ParamEditor::ParamWindow::EditParamEx& ParamEditor::ParamWindow::EditParamEx::create(C EditParam &src, MODE mode)
         {
            SCAST(EditParam, T)=src;
            switch(mode)
            {
               case CUR     : cur     =true; break;
               case BASE    : base    =true; break;
               case BASE_OVR: base_ovr=true; break;
            }
            return T;
         }
            void ParamEditor::ParamWindow::Param::GuiColor::update(C GuiPC &gpc)
{
               REPA(MT)if(MT.guiObj(i)==this && MT.bp(i))
               {
                  Param &param=*(Param*)user;
                  if(ParamEditor *pe=param.pe)
                  {
                     pe->newParam(param, false); // don't refresh gui because it may delete 'param', and what's worse we're inside param's virtual function !!
                     param.ovr.set(true, QUIET); // adjust override checkbox because we're not refreshing gui
                     param.cur=true;
                     param.setSkin();
                     param.cp.activate();
                  }
                  break;
               }
            }
            void ParamEditor::ParamWindow::Param::GuiColor::draw(C GuiPC &gpc)
{
               if(gpc.visible && visible())
               {
                  D.clip(gpc.clip);
                  Param &param=*(Param*)user;
                  Rect   rect=T.rect()+gpc.offset;
                  Color  c=param.cp(); c.a=alpha;
                  rect.draw(c, true);
                  rect.draw(Gui.borderColor(), false);
               }
            }
         void ParamEditor::ParamWindow::Param::ChangedOvr(Param &param) {if(param.ovr())param.pe->newParam(param);else param.pe->removeParam(param);}
         void ParamEditor::ParamWindow::Param::RemoveParam(Param &param) {param.pe->removeParam(param);}
         void ParamEditor::ParamWindow::Param::ClearValue(Param &param) {param.pe->clearValue (param);}
         void ParamEditor::ParamWindow::Param::ChangedType(Param &param) {param.pe->setType    (param);}
         void ParamEditor::ParamWindow::Param::ChangedName(Param &param) {param.pe->setName    (param);}
         void ParamEditor::ParamWindow::Param::ChangedCheck(Param &param) {param.pe->setBool    (param);}
         void ParamEditor::ParamWindow::Param::ChangedCombo(Param &param) {param.pe->setEnum    (param);}
         void ParamEditor::ParamWindow::Param::ChangedColor(Param &param) {param.pe->setColor   (param);}
         void ParamEditor::ParamWindow::Param::ChangedText(Param &param) {param.pe->setText    (param);}
         void ParamEditor::ParamWindow::Param::MouseEdit(Param &param)
         {
            // get total delta movement (and number of start touches)
            Vec2 d=0; int start=0; if(Gui.ms()==&param.val_adjust && Ms.b(0)){d+=Ms.d(); if(Ms.bp(0))start++; Ms.freeze();} REPA(Touches)if(Touches[i].guiObj()==&param.val_adjust && Touches[i].on()){d+=Touches[i].ad(); if(Touches[i].pd())start++;}

            if(start==1)param.mouse_edit_value=param.src.asVec2(); // on start set initial value

            if(d.any())
            {
               d*=param.mouse_edit_speed;
               if((Kb.ctrlCmd() && Kb.shift()) || (Gui.ms()==&param.val_adjust && Ms.b(1)))d.set(d.x+d.y   );else
               if(                 Kb.shift()  && param.src.type==PARAM_VEC2              )d.set(0, d.x+d.y);else
               if( Kb.ctrlCmd()                || param.src.type!=PARAM_VEC2              )d.set(d.x+d.y, 0);
               switch(param.mouse_edit_mode)
               {
                  case PROP_MOUSE_EDIT_SCALAR: {flt min=(/*(prop.md.type==DATA_REAL && prop.md.size==SIZE(Dbl)) ? 0.000000001 : */0.001f);
                                                if(Equal(param.mouse_edit_value.x, 0, min))param.mouse_edit_value.x=min;
                                                if(Equal(param.mouse_edit_value.y, 0, min))param.mouse_edit_value.y=min;
                                                param.mouse_edit_value*=Vec2(ScaleFactor(d.x), ScaleFactor(d.y));} break;
                  case PROP_MOUSE_EDIT_LINEAR: {param.mouse_edit_value+=                                       d;} break;
               }
               param.mouse_edit_value.x=param.clamp(param.mouse_edit_value.x);
               param.mouse_edit_value.y=param.clamp(param.mouse_edit_value.y);
                                        param.set  (param.mouse_edit_value  );
            }
         }
         bool ParamEditor::ParamWindow::Param::forceEnum()C {return gui && Is(gui->enum_name);}
         int  ParamEditor::ParamWindow::Param::enumValue()C {return (gui && InRange(val_combobox(), gui->name_vals)) ? gui->name_vals[val_combobox()].val : -1;}
         bool ParamEditor::ParamWindow::Param::contains(C GuiObj *obj)C
         {
            return obj==&ovr || obj==&remove || obj==&clear || obj==&type || obj==&name || obj==&val_checkbox || obj==&val_textline || obj==&val_combobox || obj==&val_color || obj==&val_adjust;
         }
         void ParamEditor::ParamWindow::Param::setSkin()
         {
            bool     exists=(cur || base);
            GuiSkinPtr skin=(exists ? null : &HalfTransparentSkin), 
                   val_skin=(valid_value ? skin : &RedSkin);

            if(!valid_type)type.skin(&RedSkin, false);else type.skin(skin, false); // if invalid then highlight as removed

            clear.skin=skin;

            if(src.hasID(Proj.list.lit_elm_id))skin=val_skin=&LitSkin;

            name.skin(skin);

            val_checkbox.skin =val_skin;
            val_textline.skin (val_skin);
            val_combobox.skin (val_skin, false); // don't change skin of menu because we only set red color here
            val_adjust  .skin =val_skin;
            val_color   .alpha=(exists ? 255 : 128);
         }
         void ParamEditor::ParamWindow::Param::toGui()
         {
            bool base=(T.base || T.base_ovr);
            valid_type=valid_value=true;
            ovr   .set(multi_cur ? false : cur, QUIET).visible( base); if(multi_cur)ovr.setMulti(); // force false for multi so next click will toggle to true
            remove.                                    visible(!base);
            type  .set(src.type==PARAM_ID_ARRAY ? PARAM_ID : src.type, QUIET).disabled(base             ).show(); // for simplicity, display PARAM_ID_ARRAY as PARAM_ID
            name  .set(                                      src.name, QUIET).disabled(base || multi_obj).show();

            bool force_enum=forceEnum();
            if(force_enum)
            {
               type.setText(gui->enum_name, true, QUIET);
               ListColumn lc[]=
               {
                  ListColumn(MEMBER(GuiEditParam::NameVal, name), LCW_DATA, "name"),
               };
               val_combobox.setColumns(lc, Elms(lc)).setData(ConstCast(gui->name_vals));
               if(multi_val)val_combobox.setText(MultipleName, true, QUIET);else{int sel=-1, value=src.asInt(); REPA(gui->name_vals)if(gui->name_vals[i].val==value){sel=i; break;} val_combobox.set(sel, QUIET);}
            }else
            switch(src.type)
            {
               default         : val_textline.clear(                                         QUIET); break;
               case PARAM_BOOL : val_checkbox.set  (                           src.value.b , QUIET); if(multi_val)val_checkbox.setMulti(); break;
               case PARAM_INT  : val_textline.set  (multi_val ? MultipleName : src.asText(), QUIET); break;
               case PARAM_FLT  : val_textline.set  (multi_val ? MultipleName : src.asText(), QUIET); break;
               case PARAM_VEC2 : val_textline.set  (multi_val ? MultipleName : src.asText(), QUIET); break;
               case PARAM_VEC  : val_textline.set  (multi_val ? MultipleName : src.asText(), QUIET); break;
               case PARAM_VEC4 : val_textline.set  (multi_val ? MultipleName : src.asText(), QUIET); break;
               case PARAM_STR  : val_textline.set  (multi_val ? MultipleName : src.asText(), QUIET); break;
               case PARAM_COLOR: if(!cp.is())cp.create("Parameter Color").func(ChangedColor, T).hide(); cp.set(multi_val ? Vec4(1) : src.value.c.asVec4(), QUIET); break;

               case PARAM_ID:
               {
                  Str text=(multi_val ? MultipleName : Proj.idToText(src.value.id, &valid_value));
                  val_textline.set(text, QUIET);
               }break;

               case PARAM_ID_ARRAY:
               {
                  Str text; if(multi_val)text=MultipleName;else FREP(src.IDs())
                  {
                     if(i)text+='|';
                     bool valid; text+=Proj.idToText(src.asID(i), &valid); valid_value&=valid;
                  }
                  val_textline.set(text, QUIET);
               }break;

               case PARAM_ENUM:
               {
                  if(src.enum_type)
                  {
                     type.setText(src.enum_type->name, true, QUIET);
                     ListColumn lc[]=
                     {
                        ListColumn(MEMBER(Enum::Elm, name), LCW_DATA, "name"),
                     };
                     val_combobox.setColumns(lc, Elms(lc)).setData(ConstCast(src.enum_type->elms()));
                     valid_type=!Proj.invalidRef(Enums.id(src.enum_type));
                  }else
                  {
                     val_combobox.clear();
                  }
                  val_combobox.setText(multi_val ? MultipleName : src.value.s, true, QUIET);
               }break;
            }
            setSkin();
            val_checkbox.visible(src.type==PARAM_BOOL );
            val_textline.visible(src.type!=PARAM_BOOL && src.type!=PARAM_ENUM && src.type!=PARAM_COLOR && !force_enum);
            val_combobox.visible(src.type==PARAM_ENUM || force_enum);
            val_color   .visible(src.type==PARAM_COLOR);
            val_adjust  .visible((src.type==PARAM_INT || src.type==PARAM_FLT || src.type==PARAM_VEC2) && !force_enum);
            Rect r=val_textline.rect(); r.max.x=(val_adjust.visible() ? val_adjust.rect().min.x : clear.rect().min.x); val_textline.rect(r); // adjust 'val_textline' rect because it depends on 'val_adjust' visibility
            mouse_edit_speed=(custom_mouse_edit_speed ? custom_mouse_edit_speed : (src.type==PARAM_INT) ? 40 : 1);

            if(!gui || !gui->desc().is()) // if there's no custom description, then adjust description depending on if it's an ID
            {
               Str desc;
               if(ParamTypeID(src.type))
               {
                  desc="Parameter Value\nDrag and drop Project Elements here or manually type an ID \"UID(..)\"\nYou can drag and drop multiple elements at the same time.\nHold Ctrl during drag and drop to add selected elements to existing value.\nCtrl+LeftClick to open the element.";
                  if(!multi_val)
                  {
                     bool count=(src.IDs()>1);
                     FREP(src.IDs())
                     {
                        desc.line(); if(count)desc+=S+i+" - "; desc+=Proj.idToText(src.asID(i));
                     }
                  }
               }else desc=MLTC(u"Parameter Value", PL, u"Wartość Parametru", DE, u"Parameter Wert", RU, u"Значение параметра", PO, u"Valor do Parâmtero");
               val_textline.desc(desc);
            }
         }
         ::ParamEditor::ParamWindow::Param& ParamEditor::ParamWindow::Param::moveToTop()
         {
            ovr         .moveToTop();
            remove      .moveToTop();
            type        .moveToTop();
            name        .moveToTop();
            val_textline.moveToTop();
            val_checkbox.moveToTop();
            val_combobox.moveToTop();
            val_color   .moveToTop();
            val_adjust  .moveToTop();
            clear       .moveToTop();
            return T;
         }
         ::ParamEditor::ParamWindow::Param& ParamEditor::ParamWindow::Param::setRect(int i, flt list_w)
         {
            flt h=elm_h,
                y=-h*i,
                w=list_w-h-0.24f-h,
                name_val_frac=0.40f,
                w_name =w*name_val_frac,
                w_value=w-w_name;
            ovr         .rect(Rect_LU(                                  0, y,       h, h));
            remove      .rect(Rect_LU(                                  0, y,       h, h));
            type        .rect(Rect_LU(remove      .rect().max.x          , y, 0.24f   , h));
            name        .rect(Rect_LU(type        .rect().max.x          , y, w_name , h));
            val_checkbox.rect(Rect_U (name        .rect().max.x+w_value/2, y,       h, h));
            val_combobox.rect(Rect_LU(name        .rect().max.x          , y, w_value, h));
            val_color   .rect(Rect_U (name        .rect().max.x+w_value/2, y,       h, h));
            clear       .rect(Rect_RU(list_w                             , y,       h, h));
            val_adjust  .rect(Rect_RU(clear       .rect().min.x          , y,       h, h));
            val_textline.rect(Rect   (name        .rect().max.x          , y-h, val_adjust.visible() ? val_adjust.rect().min.x : clear.rect().min.x, y));
            return T;
         }
         void ParamEditor::ParamWindow::Param::desc(C Str &desc)
         {
            type        .desc(desc);
            name        .desc(desc);
            val_checkbox.desc(desc);
            val_textline.desc(desc);
            val_combobox.desc(desc);
            val_color   .desc(desc);
         }
         dbl ParamEditor::ParamWindow::Param::clamp(dbl value)
         {
            if(min_use)MAX(value, min_value);
            if(max_use)MIN(value, max_value);
            return value;
         }
         void ParamEditor::ParamWindow::Param::set(C Vec2 &value)
         {
            if(pe)
            {
               Vec2 v(clamp(value.x), clamp(value.y));
               pe->newParam(T, false); // don't refresh gui because it will may delete param, and what's worse we're inside param's function !!
               ovr.set(true, QUIET); // adjust override checkbox because we're not refreshing gui
               cur=true;
               setSkin();
               val_textline.enabled(true);
               switch(src.type)
               {
                  case PARAM_INT: val_textline.set(S+Round(v.x)); break;
                  case PARAM_FLT: val_textline.set(S+      v.x ); break;
                  default       : val_textline.set(S+      v   ); break;
               }
            }
         }
         ::ParamEditor::ParamWindow::Param& ParamEditor::ParamWindow::Param::create(ParamEditor &pe, EditObject &owner, EditParamEx &src) // !! do not store pointer to 'src' because it might be temporary !!
         {
            T.pe      =&pe;
            T.owner   =&owner;
            T.id      = src.id;
            T.src     = src;
            T.cur     = src.cur;
            T.base    = src.base;
            T.base_ovr= src.base_ovr;
            pe.param_window.region+=ovr         .create(                    ).func(ChangedOvr  , T).desc(MLT("Override default value", PL,u"Zmień domyślną wartość"    , DE,u"Überschreibt Ausgangswert", RU,u"Перезаписать значение по умолчанию", PO,"Substituir o valor default"  ));
            pe.param_window.region+=remove      .create(                    ).func(RemoveParam , T).desc(MLT("Remove this parameter" , PL,u"Usuń ten parametr (Remove)", DE,"Entfernt diesen Parameter", RU,u"Удалить этот параметр"             , PO,u"Remover este parâmetro"      )); remove.image="Gui/close.img";
            pe.param_window.region+=type        .create(Proj.param_type_node).func(ChangedType , T).desc(MLT("Parameter Type"        , PL,"Typ Parametru"             , DE,"Parameter Typ"            , RU,u"Тип параметра"                     , PO,u"Tipo de Parâmetro"           )); type.set(0, QUIET); FlagDisable(type.flag, COMBOBOX_MOUSE_WHEEL);
            pe.param_window.region+=name        .create(                    ).func(ChangedName , T).desc(MLT("Parameter Name"        , PL,"Nazwa Parametru"           , DE,"Parameter Name"           , RU,u"Имя параметра"                     , PO,u"Nome do Parâmetro"           ));
            pe.param_window.region+=val_checkbox.create(                    ).func(ChangedCheck, T).focusable(false).desc(MLT("Parameter Value"       , PL,u"Wartość Parametru"         , DE,"Parameter Wert"           , RU,u"Значение параметра"                , PO,u"Valor do Parâmtero"          ));
            pe.param_window.region+=val_textline.create(                    ).func(ChangedText , T).desc(MLT("Parameter Value"       , PL,u"Wartość Parametru"         , DE,"Parameter Wert"           , RU,u"Значение параметра"                , PO,u"Valor do Parâmtero"          ));
            pe.param_window.region+=val_combobox.create(                    ).func(ChangedCombo, T).desc(MLT("Parameter Value"       , PL,u"Wartość Parametru"         , DE,"Parameter Wert"           , RU,u"Значение параметра"                , PO,u"Valor do Parâmtero"          ));
            pe.param_window.region+=val_color   .create(this                ).                      desc(MLT("Parameter Value"       , PL,u"Wartość Parametru"         , DE,"Parameter Wert"           , RU,u"Значение параметра"                , PO,u"Valor do Parâmtero"          ));
            pe.param_window.region+=val_adjust  .create(                    ).func(MouseEdit   , T).subType(BUTTON_TYPE_PROPERTY_VALUE).focusable(false); val_adjust.mode=BUTTON_CONTINUOUS;
            pe.param_window.region+=clear       .create('C'                 ).func(ClearValue  , T).desc(MLT("Clear this value"      , PL,u"Wyczyść wartość (Clear)"   , DE,u"Diesen Wert zurücksetzen" , RU,u"Очистить значение"                 , PO,"Limpar este valor"           ));
            if(gui=src.gui)
            {
               if(gui->desc().is())desc(gui->desc()); // override description
               priority=gui->priority;
               min_use=gui->min_use; min_value=gui->min_value;
               max_use=gui->max_use; max_value=gui->max_value;
               custom_mouse_edit_speed=gui->mouse_edit_speed;
                      mouse_edit_mode =gui->mouse_edit_mode;
            }
            if(pe.multi())multi_obj=true; // if 'multi' option is enabled then assume that parameter belongs to multiple objects
            return T;
         }
         void ParamEditor::ParamWindow::Param::enumChanged()
         {
            type.setData(Proj.param_type_node);
            if(src.type==PARAM_ENUM)toGui();
         }
         void ParamEditor::ParamWindow::SubObj::RemoveSubObj(SubObj &sub_obj) {sub_obj.pe->removeSubObj(sub_obj);}
         ::ParamEditor::ParamWindow::SubObj& ParamEditor::ParamWindow::SubObj::setRect(int i, flt list_w)
         {
            flt h=elm_h,
                y=-h*i,
                w=list_w-0.05f;
            remove.rect(Rect_LU(                  0, y, 0.05f, h));
            obj   .rect(Rect_LU(remove.rect().max.x, y, w   , h));
            return T;
         }
         ::ParamEditor::ParamWindow::SubObj& ParamEditor::ParamWindow::SubObj::create(ParamEditor &pe, EditObject::SubObj &src, int i, flt list_w)
         {
            T.pe=&pe;
            T.id= src.id;
            pe.param_window.region+=remove.create('R').func(RemoveSubObj, T).desc(MLT("Remove this object", PL,u"Usuń ten obiekt"  , DE,"Dieses Objekt entfernen", RU,u"Удалить этот объект"   , PO,"Remover este objecto"));
            pe.param_window.region+=obj   .create(   ).desc("Sub Object"); //.func(  EditSubObj, T).desc(MLT("Edit this object"  , PL,"Edytuj ten obiekt", DE,"Dieses Objekt editieren", RU,"Редактирование объекта", PO,"Editar este objecto" ));
            obj.text=Proj.elmFullName(src.elm_obj_id);
            if(Proj.invalidRef(src.elm_obj_id))obj.skin=&RedSkin;
            return setRect(i, list_w);
         }
      ParamEditor::ParamWindow::ParamWindow(ParamEditor &pe) : pe(pe) {}
      void     ParamEditor::ParamWindow::NewParam(ParamEditor &pe) {pe.newParam();}
      int  ParamEditor::ParamWindow::CompareParam(C Param &p0, C Param &p1) {if(int c=Compare(p0.priority, p1.priority))return c; return Compare(p0.src, p1.src);}
      ::ParamEditor::ParamWindow& ParamEditor::ParamWindow::create(C Rect &rect, bool is_class)
      {
         T.is_class=is_class;
         ::EE::Window::create("Object Class Editor").barVisible(is_class).visible(!is_class);
         if(!is_class){resize_mask=DIRF_LEFT|DIRF_DOWN; FlagDisable(flag, WIN_MOVABLE); skin(&NoShadowSkin); button[2].func(null);}else button[2].func(HideProjAct, SCAST(GuiObj, T)).show(); flag|=WIN_RESIZABLE;
         ts.reset().size=0.035f; ts.align.set(1, -1);
         T+=type   .create("Type" , &ts);
         T+=name   .create("Name" , &ts);
         T+=value  .create("Value", &ts);
         T+=new_par.create(Rect_LU(0, 0, 0.18f, 0.041f), MLTC(u"New Param", PL, u"Dodaj Parametr", DE, u"Param hinzufügen", RU, u"Добавить параметр", PO, u"Adicionar Parâmetro")).func(NewParam, pe);
         T+=region .create().slidebarSize(0.04f).skin(&LightSkin, false);
         return T.rect(rect);
      }
      Rect ParamEditor::ParamWindow::sizeLimit()C 
{
         Rect r=::EE::Window::sizeLimit();
         r.min.set(is_class ? 0.72f : 0.69f, is_class ? 0.33f : 0.15f);
         if(!is_class)r.max.set(2.0f, screenPos().y-(-D.h())-0.1f);
         return r;
      }
      flt ParamEditor::ParamWindow::listWidth() {return region.rect().w()-region.slidebarSize();}
    C Rect& ParamEditor::ParamWindow::rect()C {return ::EE::Window::rect();}
      ::ParamEditor::ParamWindow& ParamEditor::ParamWindow::rect(C Rect &rect)
{
         ::EE::Window::rect(rect);
         flt y=(is_class ? -0.16f : -0.01f);
         type .pos(Vec2(0.06f             , y));
         name .pos(Vec2(type.pos().x+0.24f, y));
         value.pos(Vec2(Lerp(name.pos().x, clientWidth()-0.10f, 0.4f), y));
         new_par.move(Vec2(clientWidth()-0.01f, y)-new_par.rect().ru());
         region.rect(Rect(0, -clientHeight(), clientWidth(), new_par.rect().min.y).extend(-0.01f));
         REPAO(  params).setRect(                i, listWidth());
         REPAO(sub_objs).setRect(params.elms()+1+i, listWidth());
         return T;
      }
      void ParamEditor::ParamWindow::update(C GuiPC &gpc)
{
         ::EE::ClosableWindow::update(gpc);
         if(!is_class && rect().h()>sizeLimit().max.y+EPS)rect(rect());
         if(Gui.window()==this)
         {
            REPA(MT)if(MT.bp(i))
            {
               REPA(params)
                  if(MT.guiObj(i)==&params[i].name && params[i].multi_obj && params[i].cur)pe.rename_window.rename(params[i].src.name, params[i].src.type, params[i].id);
            }
            if(Kb.k(KB_ENTER))REPA(params)if(Gui.kb()==&params[i].name || Gui.kb()==&params[i].val_textline) // refresh/sort on enter (this can sort by name after changing one, or resolve ID's to text after typing them manually)
            {
               toGui();
               Kb.eatKey();
               break;
            }
         }
      }
      ::ParamEditor::ParamWindow& ParamEditor::ParamWindow::hide(){if(visible()){::EE::Window::hide(); pe.paramWindowHidden();} return T;}
      ::ParamEditor::ParamWindow::Param* ParamEditor::ParamWindow::findParam(GuiObj *go)
      {
         if(contains(go))REPA(params)if(params[i].contains(go))return &params[i];
         return null;
      }
      void ParamEditor::ParamWindow::addParams(EditObject &obj)
      {
         // first get all params of single 'obj' object
         Memt<EditParamEx> obj_params;

         // current parameters first (so param.id will be set to current id)
         Memt<EditObject*> bases;
         bases.add(&obj); FREPA(obj)
         {
            EditParam &src=obj[i];
            if(!src.removed)obj_params.New().create(src, CUR);
         }
         // base parameters
         for(EditObject *base=&obj; base=base->base(); )
         {
            if(!bases.include(base))break; // if base was already processed
            FREPA(*base)
            {
               EditParam &src=(*base)[i];
               if(!src.removed)
               {
                  // find existing
                  REPA(obj_params)if(obj_params[i].name==src.name){obj_params[i].base=true; goto found_base;}
                  obj_params.New().create(src, BASE);
               found_base:;
               }
            }
         }
         // base parameters from overriden type
         if((obj.flag&EditObject::OVR_TYPE) && obj.type.valid() && obj.base) // check if that object has base (which means that it's not OBJ_CLASS)
            if(EditObjectPtr obj_class=Proj.editPath(obj.type))
               if(bases.include(obj_class()))
                  FREPA(*obj_class)
         {
            EditParam &src=(*obj_class)[i];
            if(!src.removed)
            {
               // find existing
               REPA(obj_params)if(obj_params[i].name==src.name){obj_params[i].base_ovr=true; goto found_base_ovr;}
               obj_params.New().create(src, BASE_OVR);
            found_base_ovr:;
            }
         }
         // base parameters from editor type
         if(C Memc<GuiEditParam> *edit_params=EditObjType.findParams(obj.editor_type))REPA(*edit_params)
         {
          C GuiEditParam &src=(*edit_params)[i];
            // find existing
            REPA(obj_params)if(obj_params[i].name==src.name){obj_params[i].gui=&src; obj_params[i].base=true; goto found_edit;}
            obj_params.New().create(src, BASE).gui=&src;
         found_edit:;
         }

         // now when we have object parameters, add them to the list of all params
         FREPA(obj_params)
         {
            EditParamEx &src=obj_params[i];
            REPA(params)
            {
               Param &param=params[i];
               if(ParamCompatible(param.src, src) && param.owner!=&obj) // here check also type, check only params from different object (so we won't merge multiple params of the same object)
               {
                                                    param.multi_obj =true;                   // this belongs to multiple objects and potentialy has multiple id's
                  if(        param.base!=src.base ){param.multi_base=true; param.base=true;} // have different 'base', also set 'base' to true to specify that at least one object has 'base' enabled
                  if(        param.cur !=src.cur  ){param.multi_cur =true; param.cur =true;} // have different 'cur ', also set 'cur ' to true to specify that at least one object has 'cur ' enabled
                  if(Compare(param.src , src     )) param.multi_val =true;                   // have different values
                  goto processed;
               }
            }
            params.New().create(pe, obj, src); // it was not found, so add new
         processed:;
         }
      }
      void ParamEditor::ParamWindow::addSubObjs(EditObject &obj)
      {
         int order=params.elms()+1;
         FREPA(obj.sub_objs)
         {
            EditObject::SubObj &src=obj.sub_objs[i];
            if(!src.removed)sub_objs.New().create(pe, src, order++, listWidth());
         }
      }
      void ParamEditor::ParamWindow::setSkin() {REPAO(params).setSkin();}
      void ParamEditor::ParamWindow::toGui()
      {
         Vec2 offset(region.slidebar[0].offset(), region.slidebar[1].offset());

         params  .clear();
         sub_objs.clear();

         // add params from all objects
         if(pe.world)REPA(Selection)addParams(Selection[i].params);else
         if(pe.p    )               addParams(*pe.p);
         params.sort(CompareParam);
         FREPAO(params).moveToTop().setRect(i, listWidth()).toGui(); // go from start because of 'moveToTop'

         // add sub objects
         if(pe.world){if(Selection.elms()==1)addSubObjs(Selection[0].params);}
         else                                addSubObjs(*pe.p);

         region.slidebar[0].offset(offset.x);
         region.slidebar[1].offset(offset.y);
      }
      void ParamEditor::Change::create(ptr user)
{
         if(ParamEditor *editor=(ParamEditor*)user)if(editor->p)
         {
            data=*editor->p;
            editor->undoVis();
         }
      }
      void ParamEditor::Change::apply(ptr user)
{
         if(ParamEditor *editor=(ParamEditor*)user)if(editor->p)
         {
            editor->p->undo(data, Proj.edit_path);
            editor->setChanged();
            editor->undoVis();
         }
      }
    C Str& ParamEditor::MeshVariation::originalName()C {return id ? name : S;}
      void ParamEditor::MeshVariation::set(C Str &name, uint id) {T.name=name; T.id=id;}
   void ParamEditor::undoVis() {SetUndo(undos, undo, redo);}
          void ParamEditor::multiFunc(bool Func(ParamEditor &pe, EditObject &obj, cptr    user), cptr    user)
   {
      if(multi())REPA(Proj.list.sel)if(Elm *elm=Proj.list.absToElm(Proj.list.sel[i]))if(ElmObj *data=elm->objData())if(elm!=ObjEdit.obj_elm)
      {
         ObjectPtr   game       =Proj.gamePath(elm->id); // get data of the previous version from the file
         TerrainObj2 old_terrain=*game;
         PhysPath    old_phys   =*game;
         EditObject  edit; edit.load(Proj.editPath(elm->id));
         Proj.elmChanging(*elm);
         if(Func(T, edit, user))
         {
            Save(edit, Proj.editPath(elm->id));
            data->from(edit);
            data->newVer();
            Proj.makeGameVer(*elm);
            Proj.elmChanged (*elm);
            if(old_terrain!=TerrainObj2(*game))Proj.rebuildEmbedForObj(elm->id      ); // if saving changed 'terrainObj'
            if(old_phys   !=PhysPath   (*game))Proj.rebuildPathsForObj(elm->id, true); // if saving changed 'physPath', rebuild only for objects that don't override paths (if they override then it means that changing the base doesn't affect their path mode), we must rebuild this also for objects with final path mode set to ignore, in case we've just disabled paths
            Server.setElmLong(elm->id);
         }
      }
   }
   bool ParamEditor::ChangedOConst(ParamEditor &pe, EditObject &obj, cptr user)
   {
      obj.setConst(pe.o_const(), obj.constant());
      return true;
   }
   void ParamEditor::ChangedOConst(ParamEditor &pe)
   {
      pe.setUndo("const");
      if(pe.world)REPA(Selection)ChangedOConst(pe, Selection[i].params);
      else                       ChangedOConst(pe, *pe.p);
      pe.multiFunc(ChangedOConst);
      pe.setChanged();
   }
   bool ParamEditor::ChangedConst(ParamEditor &pe, EditObject &obj, cptr user)
   {
      obj.setConst(true, pe.v_const());
      return true;
   }
   void ParamEditor::ChangedConst(ParamEditor &pe)
   {
      pe.setUndo("const");
      if(pe.world)REPA(Selection)ChangedConst(pe, Selection[i].params);
      else                       ChangedConst(pe, *pe.p);
      pe.multiFunc(ChangedConst);
      pe.setChanged();
   }
   void ParamEditor::ChangedOClass(ParamEditor &pe) // Access is merged with Type/Class
   {
      if(pe.world)
      {
         Selection.setUndo();
         REPA(Selection)
         {
            Obj &obj=Selection[i]; TerrainObj terrain=obj.terrainObj();
            obj.params.setType(pe.o_class(), obj.params.type, Proj.edit_path).setAccess(pe.o_class(), obj.params.access);
            obj.setMeshPhys(); // could have changed 'editor_type/draw_as'
            obj.setChanged(); // call before 'setChangedEmbed'
            if(obj.terrainObj()!=terrain)obj.setChangedEmbed();
         }
         pe.toGui();
      }else
      {
         // this is not available for non-world objects
      }
      ObjList.setChanged();
   }
   bool ParamEditor::ChangedClass(ParamEditor &pe, EditObject &obj, C AccessType &at)
   {
      if(at.access==OBJ_ACCESS_CUSTOM)obj.setAccess(false          ).setBase(Proj.editPath(at.type), Proj.edit_path); // use access from base
      else                            obj.setAccess(true, at.access).setBase(null                  , Proj.edit_path);
      return true;
   }
   void ParamEditor::ChangedClass(ParamEditor &pe) // Access is merged with Type/Class
   {
      AccessType at;
      bool       found=false;
                REP (ObjAccessNamesElms)if(i!=OBJ_ACCESS_CUSTOM && Equal(pe.v_class.text(), ObjAccessNames[i], true)){at.access=OBJ_ACCESS(i); found=true; break;} // don't check CUSTOM here, it will be checked below
      if(!found)REPA(Proj.existing_obj_classes)if(Elm *obj_class=Proj.findElm(Proj.existing_obj_classes[i]))if(Equal(obj_class->name, pe.v_class.text(), true)){at.access=OBJ_ACCESS_CUSTOM; at.type=obj_class->id; found=true; break;}
      if( found)
      {
         pe.setUndo("access");
         if(pe.world) // in world edit we operate on access and type only (base can be drag and dropped separately)
         {
            REPA(Selection)
            {
               Obj &obj=Selection[i]; TerrainObj terrain=obj.terrainObj();
               if(at.access==OBJ_ACCESS_CUSTOM)obj.params.setAccess(true, at.access).setType(true , at.type, Proj.edit_path);
               else                            obj.params.setAccess(true, at.access).setType(false, UIDZero, Proj.edit_path);
                                               obj.setMeshPhys(); // could have changed 'editor_type/draw_as'
                                               obj.setChanged(); // call before 'setChangedEmbed'
               if(obj.terrainObj()!=terrain   )obj.setChangedEmbed();
            }
         }else // in obj edit we operate on base and access only (type cannot be changed, it is always taken from base)
         {
            ChangedClass(pe, *pe.p, at);
            pe.setChanged(false);
         }
         pe.multiFunc(ChangedClass, at);
         pe.toGui();
         ObjList.setChanged();
      }
   }
   void ParamEditor::ChangedEditorType(ParamEditor &pe)
   {
      UID type=EditObjType.elmID(pe.v_editor_type());
      pe.setUndo("editorType");
      if(pe.world)
      {
         REPAO(Selection).params.setEditorType(type);
      }else
      {
         pe.p->setEditorType(type);
      }
      pe.setChanged();
   }
   bool ParamEditor::ChangedOPath(ParamEditor &pe, EditObject &obj, cptr user)
   {
      obj.setPath(pe.o_path(), obj.path);
      return true;
   }
   void ParamEditor::ChangedOPath(ParamEditor &pe)
   {
      pe.setUndo("path");
      if(pe.world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i]; PhysPath phys_path=obj.physPath();
            ChangedOPath(pe, obj.params);
            if(obj.physPath()!=phys_path)obj.setChangedPaths();
         }
      }else ChangedOPath(pe, *pe.p);
      pe.multiFunc(ChangedOPath);
      pe.setChanged();
   }
   bool ParamEditor::ChangedPath(ParamEditor &pe, EditObject &obj, cptr user)
   {
      obj.setPath(true, (OBJ_PATH)pe.v_path());
      return true;
   }
   void ParamEditor::ChangedPath(ParamEditor &pe)
   {
      pe.setUndo("path");
      if(pe.world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i]; PhysPath phys_path=obj.physPath();
            ChangedPath(pe, obj.params);
            if(obj.physPath()!=phys_path)obj.setChangedPaths();
         }
      }else ChangedPath(pe, *pe.p);
      pe.multiFunc(ChangedPath);
      pe.setChanged();
   }
   bool ParamEditor::ChangedOMeshVariation(ParamEditor &pe, EditObject &obj, cptr user)
   {
      obj.setMeshVariation(pe.o_mesh_var(), obj.mesh_variation_id);
      return true;
   }
   void ParamEditor::ChangedOMeshVariation(ParamEditor &pe)
   {
      pe.setUndo("meshVar");
      if(pe.world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i];
            ChangedOMeshVariation(pe, obj.params);
            obj.setMeshVariation();
            obj.setChangedEmbed();
         }
      }else ChangedOMeshVariation(pe, *pe.p);
    //pe.multiFunc(ChangedOMeshVariation); this can't be 'multi' because each object mesh has its unique mesh variation ID's
      pe.setChanged();
   }
   bool ParamEditor::ChangedMeshVariation(ParamEditor &pe, EditObject &obj, cptr user)
   {
      obj.setMeshVariation(true, InRange(pe.v_mesh_var(), pe.mesh_variations) ? pe.mesh_variations[pe.v_mesh_var()].id : 0);
      return true;
   }
   void ParamEditor::ChangedMeshVariation(ParamEditor &pe)
   {
      pe.setUndo("meshVar");
      if(pe.world)
      {
         bool has_var_name=false; Str8 var_name; if(InRange(pe.v_mesh_var(), pe.mesh_variations)){has_var_name=true; var_name=pe.mesh_variations[pe.v_mesh_var()].originalName();}
         REPA(Selection)
         {
            Obj &obj=Selection[i];
            if(obj.mesh_proper==Selection[0].mesh_proper) // process objects only with the same mesh, this is because other meshes have different variation ID's
            {
               ChangedMeshVariation(pe, obj.params);
               obj.setMeshVariation();
               obj.setChangedEmbed();
            }else
            if(has_var_name && obj.mesh_proper) // for different meshes, find by name
            {
               int var_i=obj.mesh_proper->variationFind(var_name); if(var_i>=0) // set only if a name match was found
               {
                  obj.params.setMeshVariation(true, obj.mesh_proper->variationID(var_i));
                  obj.setMeshVariation();
                  obj.setChangedEmbed();
               }
            }
         }
      }else ChangedMeshVariation(pe, *pe.p);
    //pe.multiFunc(ChangedMeshVariation); this can't be 'multi' because each object mesh has its unique mesh variation ID's
      pe.setChanged();
   }
   void ParamEditor::ToBase(ParamEditor &pe)
   {
      EditObject *params=null;
      if(pe.world)
      {
         if(Selection.elms())params=&Selection[0].params;
      }else
      {
         params=pe.p;
      }
      if(params)
      {
         if(Elm *elm=Proj.findElm(params->base.id()))switch(elm->type)
         {
            case ELM_OBJ      : ObjEdit     .activate(elm); break;
            case ELM_OBJ_CLASS: ObjClassEdit.set     (elm); break;
         }
      }
   }
   void ParamEditor::ChangedMulti(ParamEditor &edit) {edit.undo.hidden(edit.multi()); edit.redo.hidden(edit.multi()); edit.param_window.toGui();}
   void ParamEditor::Undo(ParamEditor &edit) {edit.undos.undo();}
   void ParamEditor::Redo(ParamEditor &edit) {edit.undos.redo();}
   ::ParamEditor::ParamWindow::Param* ParamEditor::findParam(GuiObj *go)
   {
      return param_window.findParam(go);
   }
   void ParamEditor::setUndo(cptr change_type, bool force_create)
   {
      if(world)
      {
         Selection.setUndo();
      }else
      {
         undos.set(change_type, force_create);
      }
   }
   void ParamEditor::setChanged(bool refresh_gui)
   {
      if(world)
      {
         REPAO(Selection).setChanged();
      }else
      {
         changed=true;
      }
      if(refresh_gui)toGui();
   }
   void ParamEditor::toGuiMeshVariation(uint mesh_variation_id)
   {
      REPA(mesh_variations)if(!i || mesh_variations[i].id==mesh_variation_id){v_mesh_var.set(i, QUIET); break;} // go from end, and if didn't found a custom variation, then stop on #0 default variation
   }
   void ParamEditor::toGui(bool params)
   {
      meshVariationChanged();
      if(world)
      {
         if(Selection.elms())
         {
            Obj        &obj_full=Selection[0];
            EditObject &obj=obj_full.params;
            bool any_custom=(obj.access==OBJ_ACCESS_CUSTOM);
            bool                                                                                mbase    =false; EditObjectPtr base    =obj.base      ;
            bool oaccess  =FlagTest(obj.flag, EditObject::OVR_ACCESS        ), moaccess  =false, maccess  =false; OBJ_ACCESS    access  =obj.access    ;
            bool otype    =FlagTest(obj.flag, EditObject::OVR_TYPE          ), motype    =false, mtype    =false; UID           type    =obj.type      ;
            bool oconst   =FlagTest(obj.flag, EditObject::OVR_CONST         ), moconst   =false, mconst   =false; bool         bconst   =obj.constant();
            bool opath    =FlagTest(obj.flag, EditObject::OVR_PATH          ), mopath    =false, mpath    =false; OBJ_PATH      path    =obj.path      ;
            bool omesh_var=FlagTest(obj.flag, EditObject::OVR_MESH_VARIATION), momesh_var=false, mmesh_var=false; uint          mesh_var=obj.mesh_variation_id; cchar8 *mesh_var_name=obj_full.variationName();
          //bool oscale   =FlagTest(obj.flag, OBJ_OVR_SCALE                ), moscale   =false, mscale   =false; Vec           scale   =obj.scale3  ()  ;
          //bool omesh    =FlagTest(obj.flag, OBJ_OVR_MESH                 ), momesh    =false, mmesh    =false; Mesh         *mesh    =obj.mesh    ()();
          //bool ophys    =FlagTest(obj.flag, OBJ_OVR_PHYS                 ), mophys    =false, mphys    =false; PhysBody     *phys    =obj.phys    ()();
          //bool omtrl    =FlagTest(obj.flag, OBJ_OVR_MATERIAL             ), momtrl    =false, mmtrl    =false; Material     *material=obj.material()();
          //bool oalign   =FlagTest(obj.flag, OBJ_OVR_ALIGN                ), moalign   =false, malign   =false; UInt          align   =(obj.alignX()|(obj.alignY()<<2)|(obj.alignZ()<<4));
            REPA(Selection)
            {
               Obj        &obj_full=Selection[i];
               EditObject &obj=obj_full.params;
               if(obj.access==OBJ_ACCESS_CUSTOM)any_custom=true;
                                                                                                                   if(base    !=obj.base             )mbase    =true;
               if(oaccess  !=FlagTest(obj.flag, EditObject::OVR_ACCESS        )){oaccess  =false; moaccess  =true;} if(access  !=obj.access           )maccess  =true;
               if(otype    !=FlagTest(obj.flag, EditObject::OVR_TYPE          )){otype    =false; motype    =true;} if(type    !=obj.type             )mtype    =true;
               if(oconst   !=FlagTest(obj.flag, EditObject::OVR_CONST         )){oconst   =false; moconst   =true;} if(bconst  !=obj.constant()       )mconst   =true;
               if(opath    !=FlagTest(obj.flag, EditObject::OVR_PATH          )){opath    =false; mopath    =true;} if(path    !=obj.path             )mpath    =true;
               if(omesh_var!=FlagTest(obj.flag, EditObject::OVR_MESH_VARIATION)){omesh_var=false; momesh_var=true;} if(mesh_var!=obj.mesh_variation_id && !Equal(mesh_var_name, obj_full.variationName()))mmesh_var=true;
             //if(oscale   !=FlagTest(obj.flag, OBJ_OVR_SCALE                )){oscale   =false; moscale   =true;} if(!Equal(scale   , obj.scale3  () ))mscale =true;
             //if(omesh    !=FlagTest(obj.flag, OBJ_OVR_MESH                 )){omesh    =false; momesh    =true;} if(       mesh    !=obj.mesh    ()())mmesh  =true;
             //if(ophys    !=FlagTest(obj.flag, OBJ_OVR_PHYS                 )){ophys    =false; mophys    =true;} if(       phys    !=obj.phys    ()())mphys  =true;
             //if(omtrl    !=FlagTest(obj.flag, OBJ_OVR_MATERIAL             )){omtrl    =false; momtrl    =true;} if(       material!=obj.material()())mmtrl  =true;
             //if(oalign   !=FlagTest(obj.flag, OBJ_OVR_ALIGN                )){oalign   =false; moalign   =true;} if(       align   !=(obj.alignX()|(obj.alignY()<<2)|(obj.alignZ()<<4)))malign=true;
             //                                                            force default |false| in above codes so next first click will toggle to true
            }
                                                                                                                          T.t_obj     .set     (mbase ? MultipleName : Proj.elmFullName(base.id()), QUIET);
            T.o_const   .set(oconst          , QUIET); if(moconst           )T.o_const   .setMulti(); if(mconst          )T.v_const   .setMulti(                         );else T.v_const.set(bconst, QUIET); T.o_const.visible(any_custom); T.v_const.visible(any_custom);
            T.o_path    .set(opath           , QUIET); if(mopath            )T.o_path    .setMulti(); if(mpath           )T.v_path    .setText (MultipleName, true, QUIET);else T.v_path .set(path  , QUIET);
            T.o_mesh_var.set(omesh_var       , QUIET); if(momesh_var        )T.o_mesh_var.setMulti(); if(mmesh_var       )T.v_mesh_var.setText (MultipleName, true, QUIET);else toGuiMeshVariation(mesh_var);
            T.o_class   .set(oaccess || otype, QUIET); if(moaccess || motype)T.o_class   .setMulti(); if(maccess || mtype)T.v_class   .setText (MultipleName, true, QUIET);else
            {
               Elm *obj_class=null; if(access==OBJ_ACCESS_CUSTOM)obj_class=Proj.findElm(type);
               if(obj_class                          )T.v_class.setText(obj_class->name        , true, QUIET);else
               if(InRange(access, ObjAccessNamesElms))T.v_class.setText(ObjAccessNames[access], true, QUIET);else T.v_class.set(-1, QUIET);
            }
          //T.oscale.set(oscale, QUIET); if(moscale)T.oscale.setMulti(); T.scale   .set(mscale ? MultipleName :       TextScale(scale   ), QUIET).disabled(!oscale && !moscale);
          //T.omesh .set(omesh , QUIET); if(momesh )T.omesh .setMulti(); T.mesh    .set(mmesh  ? MultipleName : Meshes    .name(mesh    ), QUIET);
          //T.ophys .set(ophys , QUIET); if(mophys )T.ophys .setMulti(); T.phys    .set(mphys  ? MultipleName : PhysBodies.name(phys    ), QUIET); make_phys.hide();
          //T.omtrl .set(omtrl , QUIET); if(momtrl )T.omtrl .setMulti(); T.mtrl    .set(mmtrl  ? MultipleName : Materials .name(material), QUIET);
          //T.oalign.set(oalign, QUIET); if(moalign)T.oalign.setMulti(); T.align[0].set((align&3)-1, QUIET); T.align[1].set(((align>>2)&3)-1, QUIET); T.align[2].set(((align>>4)&3)-1, QUIET);
         }
      }else
      {
         UID base_id=p->base.id(); Elm *base=Proj.findElm(base_id); if(!base)base=Proj.findElm(p->type); // just in case
         b_class.visible(base!=null);
         v_class.skin(Proj.invalidRef(base_id) ? &RedSkin : null, false); // don't change skin of menu because we only set red color here
         if(base && base->type==ELM_OBJ           )v_class.setText(Proj.elmFullName(base)  , true, QUIET);else
         if(base && base->type==ELM_OBJ_CLASS     )v_class.setText(base->name               , true, QUIET);else
         if(InRange(p->access, ObjAccessNamesElms))v_class.setText(ObjAccessNames[p->access], true, QUIET);else v_class.set(-1, QUIET);
         o_const      .set(FlagTest(p->flag, EditObject::OVR_CONST         ), QUIET).visible(p->access==OBJ_ACCESS_CUSTOM); v_const.set(p->constant(), QUIET).visible(p->access==OBJ_ACCESS_CUSTOM);
         o_path       .set(FlagTest(p->flag, EditObject::OVR_PATH          ), QUIET); v_path.set(p->path, QUIET);
         v_editor_type.set(Max(0, EditObjType.find(p->editor_type)), QUIET); // if not found then default to 0, hence "Max(0, -1)" -> 0
         o_mesh_var   .set(FlagTest(p->flag, EditObject::OVR_MESH_VARIATION), QUIET); toGuiMeshVariation(p->mesh_variation_id);
      }
      if(params)param_window.toGui();
   }
   void ParamEditor::setSkin() {param_window.setSkin();}
   void ParamEditor::create(GuiObj &parent, bool is_class, bool world)
   {
      T.world=world;
      flt y=-0.03f, h=0.04f;
      ListColumn name_desc_column[]=
      {
         ListColumn(MEMBER(NameDesc, name), LCW_MAX_DATA_PARENT, "Name"),
      };
      ts_black.reset(); ts_black.size=0.038f; ts_black.align.set(1, 0);
      ts_white.reset(); ts_white.size=0.038f; ts_white.align.set(1, 0);
      if(!is_class)
      {
         flt w=0.63f;
         parent+=::EE::Region::create().skin(&TransparentSkin, false); kb_lit=false;
         if(world)
         {
            T+=b_class.create(Rect_L(0.01f, y, 0.16f, h), "Object").func(ToBase, T).desc("Keyboard Shortctut: Ctrl+E"); T+=t_obj.create(Rect_L(0.22f, y, 0.40f, h)).disabled(true).desc("Drag and drop an object here"); y-=h;
         }else
         {
            y-=0.005f;
            T+=multi.create(Rect_L (0.010f, y         , 0.10f, 0.044f), "Multi").func(ChangedMulti, T).focusable(false).desc("When this option is enabled, all changes made to the object will additionally be applied to all currently selected project element objects.\nThis allows to modify multiple objects at the same time."); multi.mode=BUTTON_TOGGLE;
            T+=undo .create(Rect_L (0.22f , y         , 0.05f, 0.05f )).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
            T+=redo .create(Rect_LU(undo .rect().ru(), 0.05f, 0.05f )).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
            y-=0.05f;
         }
         T+=t_class   .create(Vec2(0.01f, y), "Class"    , &ts_white); if(world)T+=o_class   .create(Rect_L(0.17f, y, h, h)).func(ChangedOClass        , T).desc("Override default class"         );else T+=b_class.create(Rect_L(0.01f, y, 0.10f, h), "Class").func(ToBase, T); T+=v_class   .create(Rect_L(0.22f          , y, 0.4f, h)).func(ChangedClass        , T).desc(S+"Object Class"+(world ? "" : "\nYou can drag and drop an object here to set it as the base")); y-=h;
         T+=t_const   .create(Vec2(0.01f, y), "Const"    , &ts_white);          T+=o_const   .create(Rect_L(0.17f, y, h, h)).func(ChangedOConst        , T).desc("Override default const"         );                                                                           T+=v_const   .create(Rect_L(0.22f+(0.4f-h)/2, y,   h, h)).func(ChangedConst        , T).desc("Object Const Mode.\nThis option should be enabled if the object will not be changed during the entire game.\nWhen the option is enabled the object will not be included in the SaveGame file,\nmaking the save file smaller."); y-=h;
         T+=t_path    .create(Vec2(0.01f, y), "Path Mesh", &ts_white);          T+=o_path    .create(Rect_L(0.17f, y, h, h)).func(ChangedOPath         , T).desc("Override default path mesh"     );                                                                           T+=v_path    .create(Rect_L(0.22f          , y, 0.4f, h)).func(ChangedPath         , T).desc("Object Path Mesh Generation Mode."); v_path.setColumns(name_desc_column, Elms(name_desc_column)).setData(path_mode, Elms(path_mode)).menu.list.setElmDesc(MEMBER(NameDesc, desc)); y-=h;
         T+=t_mesh_var.create(Vec2(0.01f, y), "Variation", &ts_white);          T+=o_mesh_var.create(Rect_L(0.17f, y, h, h)).func(ChangedOMeshVariation, T).desc("Override default mesh variation");                                                                           T+=v_mesh_var.create(Rect_L(0.22f          , y, 0.4f, h)).func(ChangedMeshVariation, T).desc("Mesh Material Variation."); y-=h/2+0.01f;
         rect(Rect_LU(0, 0, w, -y));
      }
      parent+=param_window.create(is_class ? Rect(Rect_C(0, 0, 1, 1)) : Rect_RU(rect().rd(), rect().w(), 0.3f), is_class);
      if(is_class)
      {
         param_window.button[1].show();
         param_window+=t_const      .create(Vec2(0.01f, y), "Const"    , &ts_black); param_window+=v_const      .create(Rect_L(0.18f+(0.23f-h)/2, y,    h, h)).func(ChangedConst     , T).desc("Object Const Mode.\nThis option should be enabled if the object will not be changed during the entire game.\nWhen the option is enabled the object will not be included in the SaveGame file,\nmaking the save file smaller."); y-=h;
         param_window+=t_path       .create(Vec2(0.01f, y), "Path Mesh", &ts_black); param_window+=v_path       .create(Rect_L(0.18f           , y, 0.23f, h)).func(ChangedPath      , T).desc("Object Path Mesh Generation Mode."            ); v_path.setColumns(name_desc_column, Elms(name_desc_column)).setData( path_mode, Elms( path_mode)).menu.list.setElmDesc(MEMBER(NameDesc, desc)); y-=h;
         param_window+=t_editor_type.create(Vec2(0.01f, y), "Draw As"  , &ts_black); param_window+=v_editor_type.create(Rect_L(0.18f           , y, 0.23f, h)).func(ChangedEditorType, T).desc("How the object should be drawn in the editor."); v_editor_type.setColumns(name_desc_column, Elms(name_desc_column)).setData(etype_mode, Elms(etype_mode)).menu.list.setElmDesc(MEMBER(NameDesc, desc)); y-=h/2+0.01f;
         param_window+=undo         .create(Rect_LU(v_path.rect().max.x+0.05f, v_const.rect().max.y, 0.05f, 0.05f)).func(Undo, T).focusable(false).desc("Undo"); undo.image="Gui/Misc/undo.img";
         param_window+=redo         .create(Rect_LU(  undo.rect().ru()                            , 0.05f, 0.05f)).func(Redo, T).focusable(false).desc("Redo"); redo.image="Gui/Misc/redo.img";
      }
      rename_window.create();
      ListColumn lc[]=
      {
         ListColumn(MEMBER(MeshVariation, name), LCW_MAX_DATA_PARENT, "Name"),
      };
      v_mesh_var.setColumns(lc, Elms(lc), true);
   }
   void ParamEditor::enumChanged()
   {
      v_class.setData(Proj.obj_class_node);
      REPAO(param_window.params).enumChanged();
      toGui(false);
   }
   void ParamEditor::meshVariationChanged()
   {
      mesh_variations.setNum(1)[0].set("Default", 0);
      MeshPtr mesh=null;
      if(world)
      {
         if(Selection.elms())mesh=Selection[0].mesh_proper;
      }else
      {
         mesh=&ObjEdit.mesh;
         if(!OverrideMeshSkel(mesh(), ObjEdit.mesh_skel)) // if mesh is empty, then use variation ID from base
         #if 1 // this works on temporary data
            if(Elm *base_obj=Proj.findElm(p->base.id(), ELM_OBJ))
         #else // this works only on saved data
            if(Elm *obj_elm=ObjEdit.obj_elm)if(ElmObj *obj_data=obj_elm.objData())if(Elm *base_obj=Proj.findElm(obj_data.base_id, ELM_OBJ))
         #endif
               if(ElmObj *base_data=base_obj->objData())
                  mesh=Proj.gamePath(base_data->mesh_id);
      }
      if(mesh)for(int i=1; i<mesh->variations(); i++)mesh_variations.New().set(mesh->variationName(i), mesh->variationID(i));
      v_mesh_var.setData(mesh_variations, null, true);

      // for object editor refresh the gui mesh variation, because mesh is loaded after object
      if(!world)toGuiMeshVariation(p->mesh_variation_id);
   }
   void ParamEditor::objChanged(C UID *obj_id)
   {
      if(!obj_id || p->hasBase(*obj_id))toGui(); // there's no need to check if 'p' is the changed object, because 'p' will be synced in 'syncObj' with 'toGui' called there if needed
   }
   ParamEditor& ParamEditor::move(C Vec2 &delta)
{
             ::EE::Region::move(delta);
      param_window.move(rect().rd()-param_window.rect().ru());
      return T;
   }
   void ParamEditor::paramWindowHidden() {}
   void ParamEditor::newSubObj(C UID &elm_obj_id)
   {
      if(world)
      {
         if(Selection.elms()==1)
         {
            setUndo();
            Selection[0].params.sub_objs.New().elm_obj_id=elm_obj_id;
            setChanged();
         }
      }else
      {
         setUndo("newSubObj");
         p->sub_objs.New().elm_obj_id=elm_obj_id;
         setChanged();
      }
   }
   void ParamEditor::removeSubObj(ParamWindow::SubObj &sub_obj)
   {
      if(world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i];
            if(EditObject::SubObj *so=obj.params.findSubObj(sub_obj.id))if(!so->removed){obj.setUndo(); so->setRemoved(true); obj.setChanged();}
         }
         toGui();
      }else
      {
         if(EditObject::SubObj *so=p->findSubObj(sub_obj.id))if(!so->removed){undos.set("delSubObj"); so->setRemoved(true); setChanged();}
      }
   }
   bool ParamEditor::NewParam(ParamEditor &pe, EditObject &obj, cptr user)
   {
      obj.New().type=PARAM_FLT;
      return true;
   }
   void ParamEditor::newParam()
   {
      if(world)
      {
         if(Selection.elms()==1)
         {
            setUndo();
            NewParam(T, Selection[0].params);
            setChanged();
         }else
         {
            REPA(Selection)
            {
               Obj &obj=Selection[i];
               REPA(obj.params){EditParam &param=obj.params[i]; if(param.type==PARAM_FLT && !param.name.is() && !param.removed)goto already_exists;} // don't add empty flt param if object already has it
               obj.setUndo(); NewParam(T, obj.params); obj.setChanged();
            already_exists:;
            }
            toGui();
         }
      }else
      {
         setUndo("newParam");
         NewParam(T, *p);
         setChanged();
      }
      multiFunc(NewParam);
   }
   bool ParamEditor::NewParam(ParamEditor &pe, EditObject &obj, C Param &param)
   {
      return obj.newParam(param.name, param.type, Proj);
   }
   void ParamEditor::newParam(ParamWindow::Param &src, bool refresh_gui) // override 'src' parameter (this is done only if the object has a base containing that parameter)
   {
      if(world)
      {
         REPA(Selection)
         {
            EditObjectPtr obj_class; // need to keep ptr here because we're storing pointer to its parameter in 'base_param'
            Obj &obj=Selection[i];
          C EditParam *base_param=obj.params.baseFindParam(src.src.name, src.src.type);
            if(!base_param && (obj.params.flag&EditObject::OVR_TYPE))
               if(obj_class=Proj.editPath(obj.params.type))
                  base_param=obj_class->EditParams::findParam(src.src.name, src.src.type);

            if(base_param) // if base has that param
            { // copy values from 'base_param' instead of 'src' in case object bases have different default value than 'src'
               obj.setUndo();
               EditParam *dest=obj.params.EditParams::findParam(base_param->id); // find existing param with same id
               if(!dest){dest=&obj.params.New(); dest->id=base_param->id;}else if(dest->removed)dest->setRemoved(false);else continue; // if doesn't exist then create new and set id, if removed then un-remove, if exists then continue and don't override its value (this can happen if we're overriding value for multiple objects and some of which have this value already and some don't)
               SCAST(Param, *dest)=*base_param; dest->nameTypeValueUTC(); // set values and update times
               obj.setChanged();
            }
         }
      }else
      {  // no need to check for base in this case
         setUndo(&src);
         EditParam *dest=p->EditParams::findParam(src.id); // find existing param with same id
         if(!dest){dest=&p->New(); dest->id=src.id;}else if(dest->removed)dest->setRemoved(false);else return; // if doesn't exist then create new and set id, if removed then un-remove, if exists then continue and don't override its value (this can happen if we're overriding value for multiple objects and some of which have this value already and some don't)
         SCAST(Param, *dest)=src.src; dest->nameTypeValueUTC(); // set values and update times
         setChanged(false);
      }
      multiFunc(NewParam, src.src);
      if(refresh_gui)toGui();
   }
   bool ParamEditor::RemoveParam(ParamEditor &pe, EditObject &obj, C Param &src)
   {
      bool changed=false; REPA(obj)
      {
         EditParam &param=obj[i]; if(ParamCompatible(param, src) && !param.removed){param.setRemoved(true); changed=true;}
      }
      return changed;
   }
   void ParamEditor::removeParam(ParamWindow::Param &src)
   {
      if(world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i];
            REPA(obj.params){EditParam &param=obj.params[i]; if(ParamCompatible(param, src.src) && !param.removed && (src.multi_obj ? true : param.id==src.id)){obj.setUndo(); param.setRemoved(true); obj.setChanged();}}
         }
      }else
      {
         setUndo(&src);
         REPA(*p){EditParam &param=(*p)[i]; if(param.id==src.id && !param.removed)param.setRemoved(true);}
         setChanged(false);
      }
      multiFunc(RemoveParam, src.src);
      toGui();
   }
   bool ParamEditor::ClearValue(ParamEditor &pe, EditObject &obj, C Param &src)
   {
      bool changed=false; REPA(obj)
      {
         EditParam &param=obj[i]; if(ParamCompatible(param, src) && !param.removed){param.clearValue(); changed=true;}
      }
      return changed;
   }
   void ParamEditor::clearValue(ParamWindow::Param &src)
   {
      newParam(src, false); // don't refresh gui because it may make 'src' param invalid, and we will call it anyway later
      if(world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i];
            REPA(obj.params){EditParam &param=obj.params[i]; if(ParamCompatible(param, src.src) && !param.removed && (src.multi_obj ? true : param.id==src.id)){obj.setUndo(); param.clearValue(); obj.setChanged();}}
         }
      }else
      {
         setUndo(&src);
         REPA(*p){EditParam &param=(*p)[i]; if(param.id==src.id && !param.removed)param.clearValue();}
         setChanged(false);
      }
      multiFunc(ClearValue, src.src);
      toGui();
   }
   bool ParamEditor::SetType(ParamEditor &pe, EditObject &obj, C ParamWindow::Param &src)
   {
      bool       changed  =false;
      PARAM_TYPE type     =PARAM_TYPE(src.type());
      Enum      *enum_type=((type==PARAM_ENUM) ? Proj.getEnumFromName(src.type.text()) : null);
      REPA(obj)
      {
         EditParam &param=obj[i]; if(ParamCompatible(param, src.src) && !param.removed){param.setType(type, enum_type); changed=true;}
      }
      return changed;
   }
   void ParamEditor::setType(ParamWindow::Param &src)
   {
      PARAM_TYPE type     =PARAM_TYPE(src.type());
      Enum      *enum_type=((type==PARAM_ENUM) ? Proj.getEnumFromName(src.type.text()) : null);
      if(world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i];
            REPA(obj.params){EditParam &param=obj.params[i]; if(ParamCompatible(param, src.src) && !param.removed && (src.multi_obj ? true : param.id==src.id)){obj.setUndo(); param.setType(type, enum_type); obj.setChanged();}}
         }
      }else
      {
         setUndo(&src);
         REPA(*p){EditParam &param=(*p)[i]; if(param.id==src.id && !param.removed)param.setType(type, enum_type);}
         setChanged(false);
      }
      multiFunc(SetType, src);
      toGui(); // refresh because changing types may now use different gui objects, and different value
   }
   bool ParamEditor::SetName(ParamEditor &pe, EditObject &obj, C ParamWindow::Param &src)
   {
      bool changed=false;
      REPA(obj)
      {
         EditParam &param=obj[i]; if(ParamCompatible(param, src.src) && !param.removed){param.setName(src.name()); changed=true;}
      }
      return changed;
   }
   void ParamEditor::setName(ParamWindow::Param &src)
   {
      if(world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i];
            REPA(obj.params){EditParam &param=obj.params[i]; if(ParamCompatible(param, src.src) && !param.removed && (src.multi_obj ? true : param.id==src.id)){obj.setUndo(); param.setName(src.name()); obj.setChanged();}}
         }
         src.src.name=src.name(); // modify here because we're not resetting gui (and after changing world objects because they check for src.src.name)
      }else
      {
         setUndo(&src);
         REPA(*p){EditParam &param=(*p)[i]; if(param.id==src.id && !param.removed)param.setName(src.name());}
         src.src.name=src.name(); // modify here because we're not resetting gui
         setChanged(false);
      }
      multiFunc(SetName, src);
   }
   bool ParamEditor::SetBool(ParamEditor &pe, EditObject &obj, C ParamWindow::Param &src)
   {
      bool changed=false; REPA(obj)
      {
         EditParam &param=obj[i]; if(ParamCompatible(param, src.src) && !param.removed){param.setBool(src.val_checkbox()); changed=true;}
      }
      return changed;
   }
   void ParamEditor::setBool(ParamWindow::Param &src)
   {
      newParam(src, false); // don't refresh gui because it may make 'src' param invalid, and we will call it anyway later
      if(world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i];
            REPA(obj.params){EditParam &param=obj.params[i]; if(ParamCompatible(param, src.src) && !param.removed && (src.multi_obj ? true : param.id==src.id)){obj.setUndo(); param.setBool(src.val_checkbox()); obj.setChanged();}}
         }
      }else
      {
         setUndo(&src);
         REPA(*p){EditParam &param=(*p)[i]; if(param.id==src.id && !param.removed)param.setBool(src.val_checkbox());}
         setChanged(false);
      }
      multiFunc(SetBool, src);
      toGui();
   }
   bool ParamEditor::SetEnum(ParamEditor &pe, EditObject &obj, C ParamWindow::Param &src)
   {
      bool force_enum=src.forceEnum();
      int  value     =src.enumValue();
      bool changed   =false; REPA(obj)
      {
         EditParam &param=obj[i]; if(ParamCompatible(param, src.src) && !param.removed)
         {
            if(force_enum)param.setValue(value);else param.setValue(src.val_combobox.text()); changed=true;
         }
      }
      return changed;
   }
   void ParamEditor::setEnum(ParamWindow::Param &src)
   {
      newParam(src, false); // don't refresh gui because it may make 'src' param invalid, and we will call it anyway later
      bool force_enum=src.forceEnum();
      int  value     =src.enumValue();
      if(world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i];
            REPA(obj.params){EditParam &param=obj.params[i]; if(ParamCompatible(param, src.src) && !param.removed && (src.multi_obj ? true : param.id==src.id))
            {
               obj.setUndo(); if(force_enum)param.setValue(value);else param.setValue(src.val_combobox.text()); obj.setChanged();
            }}
         }
      }else
      {
         setUndo(&src);
         REPA(*p){EditParam &param=(*p)[i]; if(param.id==src.id && !param.removed)
         {
            if(force_enum)param.setValue(value);else param.setValue(src.val_combobox.text());
         }}
         setChanged(false);
      }
      multiFunc(SetEnum, src);
      toGui();
   }
   bool ParamEditor::SetColor(ParamEditor &pe, EditObject &obj, C ParamWindow::Param &src)
   {
      bool changed=false; REPA(obj)
      {
         EditParam &param=obj[i]; if(ParamCompatible(param, src.src) && !param.removed){param.setColor(src.cp()); changed=true;}
      }
      return changed;
   }
   void ParamEditor::setColor(ParamWindow::Param &src) // don't refresh gui because it will delete color pickers
   {
      src.src.value.c=src.cp(); // modify here because we're not resetting gui
      src.ovr.set(true, QUIET);
      src.cur=true;
      src.setSkin();
      if(world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i];
            REPA(obj.params){EditParam &param=obj.params[i]; if(ParamCompatible(param, src.src) && !param.removed && (src.multi_obj ? true : param.id==src.id)){obj.setUndo(); param.setColor(src.cp()); obj.setChanged();}}
         }
      }else
      {
         setUndo(&src);
         REPA(*p){EditParam &param=(*p)[i]; if(param.id==src.id && !param.removed)param.setColor(src.cp());}
         setChanged(false);
      }
      multiFunc(SetColor, src);
   }
   bool ParamEditor::SetText(ParamEditor &pe, EditObject &obj, C ParamIDs &src)
   {
      bool changed=false; REPA(obj)
      {
         EditParam &param=obj[i]; if(ParamCompatible(param, src) && !param.removed)
         {
            if(ParamTypeID(src.type))param.setAsIDArray(ConstCast(src.ids));else param.setValue(src.value.s); changed=true;
         }
      }
      return changed;
   }
   void ParamEditor::setText(ParamWindow::Param &src) // don't refresh gui because it will lose textline keyboard focus and cursor/selection
   {
      newParam(src, false); // don't refresh gui because it may make 'src' param invalid
      src.ovr.set(true, QUIET);
      src.cur=true;
      src.setSkin();
      ParamIDs data; data.name=src.src.name; data.type=src.src.type;
      if(ParamTypeID(data.type))
      {
         if(src.val_textline().is())
         {
            Memt<Str> splits; Split(splits, src.val_textline(), '|');
            FREPA(splits)
            {
               UID id;
               if(!id.fromText(splits[i]))
               if(Elm *elm=Proj.findElm(splits[i]))
               {
                  if(ElmPublish(elm->type))id=elm->id;else continue;
               }else id.zero();
               data.ids.add(id);
            }
         }
         src.src.setAsIDArray(data.ids); // modify here because we're not resetting gui
      }else
      {
         data.value.s=src.val_textline();
         src.src.setValue(data.value.s); // modify here because we're not resetting gui
      }
      if(world)
      {
         REPA(Selection)
         {
            Obj &obj=Selection[i];
            REPA(obj.params){EditParam &param=obj.params[i]; if(ParamCompatible(param, src.src) && !param.removed && (src.multi_obj ? true : param.id==src.id)){obj.setUndo(); if(ParamTypeID(param.type))param.setAsIDArray(data.ids);else param.setValue(data.value.s); obj.setChanged();}}
         }
      }else
      {
         setUndo(&src);
         REPA(*p){EditParam &param=(*p)[i]; if(param.id==src.id && !param.removed)if(ParamTypeID(param.type))param.setAsIDArray(data.ids);else param.setValue(data.value.s);}
         setChanged(false);
      }
      multiFunc(SetText, data);
   }
   bool ParamEditor::SetBase(ParamEditor &pe, EditObject &obj, C EditObjectPtr &base)
   {
      obj.setBase(base, Proj.edit_path).setAccess(false); // use access from base
      return true;
   }
   void ParamEditor::setBase(C EditObjectPtr &base)
   {
      if(world)
      {
         setUndo();
         REPA(Selection)
         {
            Obj &obj=Selection[i]; TerrainObj terrain=obj.terrainObj(); PhysPath phys_path=obj.physPath(); uint mesh_variation_id=obj.params.mesh_variation_id;
            obj.params.setBase(base, Proj.edit_path); // don't modify access because in WorldEditor we can modify it manually
            obj.setMeshPhys();
            obj.setChanged(); // call before 'setChangedEmbed'
            if(obj.terrainObj()!=terrain || obj.params.mesh_variation_id!=mesh_variation_id)obj.setChangedEmbed(); // call after 'setChanged' and before 'setChangedPaths'
            if(obj.physPath()  !=phys_path                                                 )obj.setChangedPaths(); // call after 'setChangedEmbed'
         }
         ObjList.setChanged();
      }else
      {
         setUndo("setBase");
         SetBase(T, *p, base);
         setChanged(false);
      }
      multiFunc(SetBase, base);
      toGui();
   }
   bool ParamEditor::SetID(ParamEditor &pe, EditObject &obj, C ParamIDs &src)
   {
      bool changed=false; REPA(obj)
      {
         EditParam &param=obj[i]; if(ParamCompatible(param, src) && !param.removed){param.setAsIDArray(ConstCast(src.ids)); changed=true;}
      }
      return changed;
   }
   bool ParamEditor::IncludeID(ParamEditor &pe, EditObject &obj, C ParamIDs &src)
   {
      bool changed=false; REPA(obj)
      {
         EditParam &param=obj[i]; if(ParamCompatible(param, src) && !param.removed){param.includeAsIDArray(ConstCast(src.ids)); changed=true;}
      }
      return changed;
   }
   void ParamEditor::drag(Memc<UID> &elms, GuiObj* &obj, C Vec2 &screen_pos)
   {
      if(elms.elms() && (contains(obj) || param_window.contains(obj)))
      {
         if(param_window.contains(obj)) // set project element as param id or sub object
         {
            if(ParamWindow::Param *src=findParam(obj))
            {
               bool include=Kb.ctrl(); // if Ctrl pressed then include ID's
               Memt<UID> publishable; FREPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(ElmPublish(elm->type))publishable.add(elm->id);
               if(publishable.elms())
                  if(!src->base || ParamTypeID(src->src.type)) // only if there's no base, or it's of PARAM_ID* type
               {
                  newParam(*src, false); // don't refresh gui because it may make 'src' param invalid, and we will call it anyway later
                  if(world)
                  {
                     REPA(Selection)
                     {
                        Obj &obj=Selection[i];
                        REPA(obj.params){EditParam &param=obj.params[i]; if(ParamCompatible(param, src->src) && !param.removed && (src->multi_obj ? true : param.id==src->id)){obj.setUndo(); if(include)param.includeAsIDArray(publishable);else param.setAsIDArray(publishable); obj.setChanged();}}
                     }
                  }else
                  {
                     REPA(*p){EditParam &param=(*p)[i]; if(param.id==src->id && !param.removed){setUndo(src); if(include)param.includeAsIDArray(publishable);else param.setAsIDArray(publishable); setChanged(false); break;}}
                  }
                  ParamIDs data; data.name=src->src.name; data.type=src->src.type; data.ids=publishable;
                  multiFunc(include ? IncludeID : SetID, data);
                  toGui();
               }
            }else
            REPA(elms)if(Elm *elm=Proj.findElm(elms[i], ELM_OBJ))
            {
               newSubObj(elm->id);
               break; // allow adding only one object at a time, in case user accidentally dragged entire project
            }
         }else // set object base
         {
          //if(obj==&t_obj || v_class.contains(obj)) any place is good to set the base
            {
               undos.set("setBase");
               FREPA(elms)if(Elm *elm=Proj.findElm(elms[i]))if(elm->type==ELM_OBJ || (elm->type==ELM_OBJ_CLASS && !world)) // for world objects we can't set obj.base as ELM_OBJ_CLASS
               {
                  EditObjectPtr base=Proj.editPath(elm->id); setBase(base);
                  break;
               }
               ObjList.setChanged();
            }
         }
         obj=null; // clear in case got deleted, this is very important, because if the obj was pointing to a parameter that due to 'toGui' got deleted, then 'obj' would become an invalid pointer and accessing it later could cause a crash
      }
   }
   void ObjClassEditor::Locate(ObjClassEditor &editor) {Proj.elmLocate(editor.elm_id);}
   void ObjClassEditor::paramWindowHidden(){set(null);}
   void ObjClassEditor::create()
   {
      ::ParamEditor::create(*Gui.desktop(), true);
      param_window+=locate.create(Rect_LU(redo.rect().ru()+Vec2(0.01f, 0), 0.14f, 0.05f), "Locate").func(Locate, T).focusable(false).desc("Locate this element in the Project");
   }
   void ObjClassEditor::flush()
   {
      if(elm && changed)
      {
         if(ElmObjClass *data=elm->objClassData())
         {
            OBJ_PATH path=data->pathSelf();
            data->newVer(); data->from(*p); // modify just before saving/sending in case we've received data from server after edit
            if(path!=data->pathSelf())Proj.verifyPathsForObjClass(elm->id); // if paths were changed
         }
         Save(*p, Proj.editPath(elm->id));
         Object obj; p->copyTo(obj, Proj, false, null, null); Save(obj, Proj.gamePath(elm->id)); Proj.savedGame(*elm);
         Server.setElmLong(elm->id);
         Proj.objChanged(*elm); // call 'Proj.objChanged' instead of 'Proj.elmChanged' so 'ObjClassEdit.elmChanged' won't get called
      }
      changed=false;
   }
   void ObjClassEditor::set(Elm *elm)
   {
      if(elm && elm->type!=ELM_OBJ_CLASS)elm=null;
      if(T.elm!=elm)
      {
         rename_window.hide();
         flush();
         undos.del(); undoVis();
         T.elm   =elm;
         T.elm_id=(elm ? elm->id : UIDZero);
         if(elm)p->load(Proj.editPath(elm->id));else p->del();
         toGui();
         Proj.refresh(false, false);
         param_window.visible(T.elm!=null).moveToTop();
      }
   }
   void ObjClassEditor::activate(Elm *elm) {set(elm); if(T.elm)::EE::GuiObj::activate();}
   void ObjClassEditor::toggle(Elm *elm) {if(elm==T.elm)elm=null; set(elm);}
   void ObjClassEditor::elmChanged(C UID &elm_id)
   {
      if(elm_id==T.elm_id && elm_id.valid())
      {
         undos.set(null, true);
         EditObject temp; if(temp.load(Proj.editPath(elm_id)))if(p->sync(temp, Proj.edit_path))toGui();
      }
   }
   void ObjClassEditor::erasing(C UID &elm_id) {if(elm && elm->id==elm_id)set(null);}
ParamEditor::ParamEditor() : param_window(T), rename_window(T), world(false), p(&temp_p), changed(false), undos(true, this) {}

ParamEditor::ParamWindow::EditParamEx::EditParamEx() : cur(false), base(false), base_ovr(false), gui(null) {}

ParamEditor::ParamWindow::Param::Param() : cur(false), base(false), base_ovr(false), valid_type(true), valid_value(true), multi_obj(false), multi_cur(false), multi_base(false), multi_val(false), priority(0), id(UIDZero), pe(null), owner(null), min_use(false), max_use(false), min_value(0    ), max_value(0), mouse_edit_speed(1), custom_mouse_edit_speed(0), mouse_edit_value(0), gui(null), mouse_edit_mode(PROP_MOUSE_EDIT_LINEAR) {}

ParamEditor::ParamWindow::Param::GuiColor::GuiColor() : alpha(255) {}

ParamEditor::ParamWindow::SubObj::SubObj() : id(UIDZero), pe(null) {}

ParamEditor::MeshVariation::MeshVariation() : id(0) {}

ParamEditor::AccessType::AccessType() : type(UIDZero) {}

ObjClassEditor::ObjClassEditor() : elm(null), elm_id(UIDZero) {}

/******************************************************************************/
