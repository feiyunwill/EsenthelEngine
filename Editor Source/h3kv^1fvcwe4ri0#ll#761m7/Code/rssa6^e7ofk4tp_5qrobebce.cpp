/******************************************************************************/
class EditParam : Param
{
   UID       id=UID().randomizeValid();
   bool      removed=false;
   TimeStamp removed_time, name_time, type_time, value_time;

   // get
   bool old(C TimeStamp &now=TimeStamp().getUTC())C
   {
      return removed_time<now && name_time<now && type_time<now && value_time<now;
   }
   bool equal(C EditParam &src)C
   {
      return removed_time==src.removed_time && name_time==src.name_time && type_time==src.type_time && value_time==src.value_time;
   }
   bool newer(C EditParam &src)C
   {
      return removed_time>src.removed_time || name_time>src.name_time || type_time>src.type_time || value_time>src.value_time;
   }
   uint memUsage()C {return super.memUsage();}
   bool contains(C Str &text, Project &proj)C
   {
      if(Contains(name, text))return true;
      if(type==PARAM_STR && Contains(value.s, text))return true;
      if(ParamTypeID(type))REP(IDs())if(Contains(proj.elmFullName(asID(i)), text))return true;
      return false;
   }

   // operations
   EditParam& create(C Param &src, C TimeStamp &time=TimeStamp().getUTC())
   {
      SCAST(Param, T)=src;
      removed=false;
      removed_time=name_time=type_time=value_time=time;
      return T;
   }
   void       setRemoved      (  bool   removed                     ) {T.removed=removed; removed_time.getUTC();}
   EditParam& setName         (C Str   &name                        ) {T.name   =name   ;    name_time.getUTC(); return T;}
   void       clearValue      (                                     ) {if(type==PARAM_ID)setAsIDArray(null);else{super.clearValue(); value_time.getUTC();}}
   void       setBool         (  bool   v                           ) {value.b=v;                       value_time.getUTC();}
   void       setColor        (C Color &v                           ) {value.c=v;                       value_time.getUTC();}
   void       setValue        (  int    v                           ) {super.setValue(v);               value_time.getUTC();}
   void       setValue        (C Str   &v                           ) {super.setValue(v);               value_time.getUTC();}
   EditParam& forceBool       (  bool   v                           ) {setType(PARAM_BOOL ); value.b=v; value_time.getUTC(); return T;}
   EditParam& forceInt        (  int    v                           ) {setType(PARAM_INT  ); value.i=v; value_time.getUTC(); return T;}
   EditParam& forceFlt        (  flt    v                           ) {setType(PARAM_FLT  ); value.f=v; value_time.getUTC(); return T;}
   EditParam& forceColor      (C Color &v                           ) {setType(PARAM_COLOR); value.c=v; value_time.getUTC(); return T;}
   EditParam&     setAsIDArray(C MemPtr<UID> &ids                   ) {super.    setAsIDArray(ids); type_time.getUTC(); value_time.getUTC(); return T;}
   EditParam& includeAsIDArray(C MemPtr<UID> &ids                   ) {super.includeAsIDArray(ids); type_time.getUTC(); value_time.getUTC(); return T;}
   EditParam& setType         (PARAM_TYPE type, Enum *enum_type=null) {super.setType((type==PARAM_ID) ? PARAM_ID_ARRAY : type, enum_type); type_time.getUTC(); return T;}
   EditParam& setTypeValue    (C Param &src                         ) {super.setTypeValue(src); type_time.getUTC(); value_time.getUTC(); return T;}
   void       nameTypeValueUTC(                                     ) {name_time.getUTC();      type_time.getUTC(); value_time.getUTC();}

   bool sync(C EditParam &src)
   {
      bool changed=false;
      changed|=Sync(removed_time, src.removed_time, removed, src.removed);
      changed|=Sync(   name_time, src.   name_time, name   , src.name   );
      if(Sync(type_time, src.type_time)) // adjust type first before the value
      {
         changed=true;
         super.setType(src.type, src.enum_type);
      }
      if(Sync(value_time, src.value_time))
      {
         changed=true;
         super.setValue(src);
      }
      return changed;
   }
   bool undo(C EditParam &src)
   {
      bool changed=false;
      changed|=Undo(removed_time, src.removed_time, removed, src.removed);
      changed|=Undo(   name_time, src.   name_time, name   , src.name   );
      if(Undo(type_time, src.type_time)) // adjust type first before the value
      {
         changed=true;
         super.setType(src.type, src.enum_type);
      }
      if(Undo(value_time, src.value_time))
      {
         changed=true;
         super.setValue(src);
      }
      return changed;
   }

   // io
   bool save(File &f, cchar *game_path=null)C
   {
      f.cmpUIntV(0);
      f<<id<<removed<<removed_time<<name_time<<type_time<<value_time;
      super.save(f, game_path);
      return f.ok();
   }
   bool load(File &f, cchar *game_path=null)
   {
      switch(f.decUIntV())
      {
         case 0:
         {
            f>>id>>removed>>removed_time>>name_time>>type_time>>value_time;
            if(super.load(f, game_path))
               if(f.ok())return true;
         }break;
      }
      return false;
   }
}
/******************************************************************************/
class EditParams : Memc<EditParam>
{
   // get
   EditParam* findParam(C UID &id                   )  {REPA(T){EditParam &p=T[i]; if( p.id==id                                                      )return &p;} return null;}
   EditParam* findParam(C Str &name                 )  {REPA(T){EditParam &p=T[i]; if(!p.removed && p.name==name                                     )return &p;} return null;}
   EditParam* findParam(C Str &name, PARAM_TYPE type)  {REPA(T){EditParam &p=T[i]; if(!p.removed && p.name==name && ParamTypeCompatible(p.type, type))return &p;} return null;}
 C EditParam* findParam(C UID &id                   )C {return ConstCast(T).findParam(id);}
 C EditParam* findParam(C Str &name                 )C {return ConstCast(T).findParam(name);}
 C EditParam* findParam(C Str &name, PARAM_TYPE type)C {return ConstCast(T).findParam(name, type);}

   EditParam* findParamInclRemoved(C Str &name                 ) {EditParam *ret=null; REPA(T){EditParam &p=T[i]; if(p.name==name                                     ){ret=&p; if(!p.removed)break;}} return ret;} // prioritize non-removed first
   EditParam* findParamInclRemoved(C Str &name, PARAM_TYPE type) {EditParam *ret=null; REPA(T){EditParam &p=T[i]; if(p.name==name && ParamTypeCompatible(p.type, type)){ret=&p; if(!p.removed)break;}} return ret;} // prioritize non-removed first

   uint memUsage()C
   {
      uint   size=0; REPA(T)size+=T[i].memUsage();
      return size;
   }

   bool old(C TimeStamp &now=TimeStamp().getUTC())C
   {
      REPA(T)if(!T[i].old(now))return false;
      return true;
   }
   bool equal(C EditParams &src)C
   {
      if(elms()==src.elms())
      {
         REPA(T)
         {
          C EditParam &param=T[i], *src_param=src.findParam(param.id);
            if(!src_param || !param.equal(*src_param))return false;
         }
         return true;
      }
      return false;
   }
   bool newer(C EditParams &src)C
   {
      REPA(T)
      {
       C EditParam &param=T[i], *src_param=src.findParam(param.id);
         if(!src_param || param.newer(*src_param))return true;
      }
      return false;
   }

   // operations
   void create(C EditParams &src)
   {
      FREPA(src)if(!src[i].removed)add(src[i]);
   }
   void create(C Object &src, C TimeStamp &time=TimeStamp().getUTC())
   {
      clear(); FREPA(src.params)New().create(src.params[i], time);
   }
   void copyTo(Object &obj)C
   {
      obj.params.del(); FREPA(T)if(!T[i].removed)obj.params.New()=T[i]; obj.sortParams();
   }
   bool sync(C EditParams &src)
   {
      bool changed=false;
      FREPA(T) // sync same params
      {
         EditParam &param=T[i];
         if(C EditParam *src_param=src.findParam(param.id))changed|=param.sync(*src_param);
      }
      FREPA(src) // add new params from src
      {
       C EditParam &src_param=src[i];
         if(!findParam(src_param.id)){changed=true; add(src_param);}
      }
      return changed;
   }
   bool undo(C EditParams &src)
   {
      bool changed=false;
      // mark as removed those that aren't present in 'src'
      REPA(T)
      {
         EditParam &param=T[i]; if(!param.removed && !src.findParam(param.id)){param.removed=true; param.removed_time++; changed=true;}
      }
      // undo/insert from 'src'
      REPA(src)
      {
         C EditParam &s=src[i];
           EditParam *t=findParam(s.id);
         if(t)changed|=t.undo(s);else{add(s); changed=true;}
      }
      return changed;
   }

   // io
   bool save(File &f, cchar *game_path=null)C
   {
      f.cmpUIntV(0);
      return super.save(f, game_path);
   }
   bool load(File &f, cchar *game_path=null)
   {
      switch(f.decUIntV())
      {
         case 0: return super.load(f, game_path);
      }
      return false;
   }
}
/******************************************************************************/
class EditObject : EditParams
{
   class SubObj
   {
      bool      removed=false;
      UID       id=UID().randomizeValid(), // id of the sub object instance
                elm_obj_id=UIDZero; // project ELM_OBJ element id
      TimeStamp removed_time;

      // get
      bool old  (C TimeStamp &now=TimeStamp().getUTC())C {return removed_time<now;}
      bool equal(C SubObj    &src                     )C {return removed_time==src.removed_time;}
      bool newer(C SubObj    &src                     )C {return removed_time> src.removed_time;}

      // operations
      void setRemoved(bool removed) {T.removed=removed; removed_time.getUTC();}

      bool sync(C SubObj &src) {return Sync(removed_time, src.removed_time, removed, src.removed);}
      bool undo(C SubObj &src) {return Undo(removed_time, src.removed_time, removed, src.removed);}

      // io
      bool save(File &f)C
      {
         f.cmpUIntV(0);
         f<<removed<<removed_time<<id<<elm_obj_id;
         return f.ok();
      }
      bool load(File &f)
      {
         switch(f.decUIntV())
         {
            case 0:
            {
               f>>removed>>removed_time>>id>>elm_obj_id;
               if(f.ok())return true;
            }break;
         }
         return false;
      }
   }

   enum FLAG
   {
      OVR_ACCESS        =1<<0,
      OVR_TYPE          =1<<1,
      OVR_CONST         =1<<2,
      OVR_PATH          =1<<3,
      CONSTANT          =1<<4,
      OVR_MESH_VARIATION=1<<5,
   }

   byte          flag=0; // FLAG
   OBJ_ACCESS    access=OBJ_ACCESS_TERRAIN;
   OBJ_PATH      path  =OBJ_PATH_CREATE;
   uint          mesh_variation_id=0;
   UID           type=UIDZero, // elm.id of ELM_OBJ_CLASS
                 editor_type=UIDZero; // how to draw the object in the editor
   TimeStamp     base_time, access_time, type_time, editor_type_time, const_time, path_time, mesh_variation_time;
   EditObjectPtr base; // this can be set to ELM_OBJ and ELM_OBJ_CLASS
   Memc<SubObj>  sub_objs;

   bool constant()C {return FlagTest(flag, CONSTANT);}

   uint memUsage()C {return super.memUsage()+sub_objs.memUsage();}

   SubObj* findSubObj(C UID &id)  {REPA(sub_objs)if(sub_objs[i].id==id)return &sub_objs[i]; return null;}
 C SubObj* findSubObj(C UID &id)C {return ConstCast(T).findSubObj(id);}

   void del()
   {
      super.del();
      flag=0;
      type.zero();
      editor_type.zero();
      access=OBJ_ACCESS_TERRAIN;
      path  =OBJ_PATH_CREATE;
      mesh_variation_id=0;
      base_time=access_time=type_time=editor_type_time=const_time=path_time=mesh_variation_time=0;
      base=null;
   }

   // get / set
 C EditParam* findParam(C Str &name)C
   {
      Memt<C EditObject*> bases;
      for(C EditObject *cur=this; cur && bases.include(cur); cur=cur.base())if(C EditParam *param=cur.EditParams.findParam(name))return param; // check self and bases
      if(C Memc<GuiEditParam> *params=EditObjType.findParams(editor_type))REPA(*params)if((*params)[i].name==name)return &(*params)[i]; // check editor type
      return null;
   }
 C EditParam* baseFindParam(C Str &name, PARAM_TYPE type)C
   {
      Memt<C EditObject*> bases;
      for(C EditObject *cur=base(); cur && bases.include(cur); cur=cur.base())if(C EditParam *param=cur.EditParams.findParam(name, type))return param; // check bases (skip self)
      if(C Memc<GuiEditParam> *params=EditObjType.findParams(editor_type))REPA(*params)if((*params)[i].name==name && ParamTypeCompatible((*params)[i].type, type))return &(*params)[i]; // check editor type
      return null;
   }
   bool paramValuesContain(C Str &text, Project &proj)C
   {
      Memt<C EditObject*> bases;
      for(C EditObject *cur=this; cur && bases.include(cur); cur=cur.base()) // check self and bases
      {
         REPA(*cur) // iterate all params of this base
            if(C EditParam *param=findParam((*cur)[i].name)) // use 'this' for 'findParam' (instead of 'cur') because we want to get only overriden values
               if(param.contains(text, proj))return true;
      }
      if(C Memc<GuiEditParam> *params=EditObjType.findParams(editor_type)) // check editor type
      {
         REPA(*params)
            if(C EditParam *param=findParam((*params)[i].name)) // use 'this' for 'findParam' (instead of 'params') because we want to get only overriden values
               if(param.contains(text, proj))return true;
      }
      return false;
   }

   bool hasBase(C UID &obj_id)C
   {
      if(obj_id.valid())
      {
         Memt<C EditObject*> bases;
         for(C EditObject *cur=this; cur && bases.include(cur); cur=cur.base()) // check self and bases
            if(EditObjects.id(cur)==obj_id)return true;
      }
      return false;
   }
   EditObject& setBase(C EditObjectPtr &base, cchar *edit_path)
   {
      base_time.getUTC();
      T.base=base;
      updateBase(edit_path);
      return T;
   }
   EditObject& setAccess(bool custom, OBJ_ACCESS access=OBJ_ACCESS_TERRAIN)
   {
      access_time.getUTC();
      FlagSet(flag, OVR_ACCESS, custom);
      T.access=(custom ? access : base ? base->access : OBJ_ACCESS_TERRAIN);
      return T;
   }
   EditObject& setType(bool custom, C UID &type, cchar *edit_path)
   {
      type_time.getUTC();
      FlagSet(flag, OVR_TYPE, custom);
      if(custom)T.type=type;else if(base)T.type=base->type;else T.type.zero();
      updateEditorType(edit_path);
      return T;
   }
   EditObject& setEditorType(C UID &type=UIDZero)
   {
      T.editor_type_time.getUTC();
      T.editor_type     =type;
      return T;
   }
   EditObject& setConst(bool custom, bool on=false)
   {
      const_time.getUTC();
      FlagSet(flag, OVR_CONST, custom);
      FlagSet(flag, CONSTANT, custom ? on : base ? base->constant() : false);
      return T;
   }
   EditObject& setPath(bool custom, OBJ_PATH path=OBJ_PATH_CREATE)
   {
      path_time.getUTC();
      FlagSet(flag, OVR_PATH, custom);
      T.path=(custom ? path : base ? base->path : OBJ_PATH_CREATE);
      return T;
   }
   EditObject& setMeshVariation(bool custom, uint mesh_variation_id=0)
   {
      mesh_variation_time.getUTC();
      FlagSet(flag, OVR_MESH_VARIATION, custom);
      T.mesh_variation_id=(custom ? mesh_variation_id : base ? base->mesh_variation_id : 0);
      return T;
   }

   bool subObjsOld  (C TimeStamp &now=TimeStamp().getUTC())C {REPA(sub_objs)if(!sub_objs[i].old(now))return false; return true;}
   bool subObjsEqual(C EditObject &src)C
   {
      if(sub_objs.elms()==src.sub_objs.elms())
      {
         REPA(sub_objs)
         {
            C SubObj &obj=sub_objs[i], *src_obj=src.findSubObj(obj.id);
            if(!src_obj || !obj.equal(*src_obj))return false;
         }
         return true;
      }
      return false;
   }
   bool subObjsNewer(C EditObject &src)C
   {
      REPA(sub_objs)
      {
         C SubObj &obj=sub_objs[i], *src_obj=src.findSubObj(obj.id);
         if(!src_obj || obj.newer(*src_obj))return true;
      }
      return false;
   }

   bool old  (C TimeStamp &now=TimeStamp().getUTC())C {return base_time< now           && access_time< now             && type_time< now           && editor_type_time< now                  && const_time< now            && path_time< now           && mesh_variation_time< now                     && super.old  (now) && subObjsOld  (now);}
   bool equal(C EditObject &src                    )C {return base_time==src.base_time && access_time==src.access_time && type_time==src.type_time && editor_type_time==src.editor_type_time && const_time==src.const_time && path_time==src.path_time && mesh_variation_time==src.mesh_variation_time && super.equal(src) && subObjsEqual(src);}
   bool newer(C EditObject &src                    )C {return base_time> src.base_time || access_time> src.access_time || type_time> src.type_time || editor_type_time> src.editor_type_time || const_time> src.const_time || path_time> src.path_time || mesh_variation_time> src.mesh_variation_time || super.newer(src) || subObjsNewer(src);}

   // operations
   void newData()
   {
      base_time++; access_time++; type_time++; const_time++; path_time++; mesh_variation_time++;
    //editor_type_time++; don't set this
   }
   void create(C EditObject &src)
   {
      super.create(src);
      flag             =src.flag             ;          const_time=src.         const_time;
      access           =src.access           ;         access_time=src.        access_time;
      path             =src.path             ;           path_time=src.          path_time;
      type             =src.type             ;           type_time=src.          type_time;
      base             =src.base             ;           base_time=src.          base_time;
      mesh_variation_id=src.mesh_variation_id; mesh_variation_time=src.mesh_variation_time;
      editor_type=src.editor_type;
   }
   void create(C Object &src, C UID &type, C EditObjectPtr &base, C TimeStamp &time=TimeStamp().getUTC())
   {
      super.create(src, time);
      base_time=access_time=type_time=const_time=path_time=mesh_variation_time=time;
      FlagSet(flag, OVR_ACCESS        , src.customAccess       ());
      FlagSet(flag, OVR_TYPE          , src.customType         ());
      FlagSet(flag, OVR_PATH          , src.customPath         ());
      FlagSet(flag, OVR_CONST         , src.customConstant     ());
      FlagSet(flag, OVR_MESH_VARIATION, src.customMeshVariation());
      FlagSet(flag, CONSTANT          , src.constant           ());
      access           =src.access         ();
      path             =src.path           ();
      mesh_variation_id=src.meshVariationID();
      T.type=type;
      T.base=base;
      editor_type.zero();
      editor_type_time.zero(); // set this to zero because depending on this we're detecting if it was manually set
   }
   void copyTo(Object &obj, C Project &proj, bool zero_align, C UID *mesh_id, C UID *phys_id)C // 'zero_align' should be set only to ELM_OBJ
   {
      super.copyTo(obj);
      obj.access         (FlagTest(flag, OVR_ACCESS        ), access           );
      obj.type           (FlagTest(flag, OVR_TYPE          ), type             );
      obj.constant       (FlagTest(flag, OVR_CONST         ), constant()       );
      obj.path           (FlagTest(flag, OVR_PATH          ), path             );
      obj.meshVariationID(FlagTest(flag, OVR_MESH_VARIATION), mesh_variation_id);
      obj.align          (zero_align);
      UID base_id=base.id();           obj.base(      proj.gamePath( base_id));
      if(!mesh_id)obj.mesh(false);else obj.mesh(true, proj.gamePath(*mesh_id));
      if(!phys_id)obj.phys(false);else obj.phys(true, proj.gamePath(*phys_id));
      FREPA(sub_objs){C SubObj &src=sub_objs[i]; if(!src.removed)obj.sub_objs.New().base(proj.gamePath(src.elm_obj_id));}
   }
   void copyTo(Edit.ObjData &obj, bool include_removed_params)C
   {
      obj.elm_obj_class_id=type;
      obj.          access=access;
      obj.            path=path;
      obj.params.clear();
      Memt<C EditObject*> bases;
      bool inherited=false;
      for(C EditObject *cur=this; cur && bases.include(cur); cur=cur.base(), inherited=true) // start from self, go to base/parents
         FREPA(*cur) // list in order
      {
       C EditParam &s=(*cur)[i];
         if(include_removed_params || !s.removed)
         {
            Edit.ObjData.Param &d=obj.params.New();
            SCAST(EE.Param, d)=s;
            d.id       =s.id;
            d.removed  =s.removed;
            d.inherited=inherited;
         }
      }
   }
   void updateEditorType(cchar *edit_path)
   {
      if(!editor_type_time.is()) // if this is not a manually set value (this can occur only for OBJ_CLASS), this check is important because we can't adjust the 'editor_type' of OBJ_CLASS in this method
      {
         if(base) // if we have a base
         {
            if(FlagTest(flag, OVR_TYPE)) // we must get 'editor_type' from custom 'type' (check this only inside 'base' condition, because this will mean that it's a world object, and only then we're interested in 'OVR_TYPE', because otherwise 'OVR_TYPE' is set only for 'OBJ_CLASS' for which we can't adjust 'editor_type')
            {
               EditObjectPtr type_base; if(type.valid())type_base=Str(edit_path).tailSlash(true)+EncodeFileName(type);
               if(type_base)editor_type=type_base->editor_type;else editor_type.zero();
            }else
               editor_type=base->editor_type;
         }else editor_type.zero();
      }
   }
   void updateBase(cchar *edit_path)
   {
      if(!FlagTest(flag, OVR_ACCESS        ))access=(base ? base->access : OBJ_ACCESS_TERRAIN);
      if(!FlagTest(flag, OVR_TYPE          ))if(base)type=base->type;else type.zero();
      if(!FlagTest(flag, OVR_CONST         ))FlagSet(flag, CONSTANT, base ? base->constant() : false);
      if(!FlagTest(flag, OVR_PATH          ))path=(base ? base->path : OBJ_PATH_CREATE);
      if(!FlagTest(flag, OVR_MESH_VARIATION))mesh_variation_id=(base ? base->mesh_variation_id : 0);
      updateEditorType(edit_path);
   }
   bool newParam(C Str &name, PARAM_TYPE type, C Project &proj)
   {
      EditObjectPtr obj_class; // need to keep ptr here because we're storing pointer to its parameter in 'base_param'
    C EditParam    *base_param=baseFindParam(name, type);
      if(!base_param && (flag&OVR_TYPE))
         if(obj_class=proj.editPath(T.type))
            base_param=obj_class->EditParams.findParam(name, type);

      if(base_param)
      {
         EditParam *dest=EditParams.findParam(base_param.id); // find existing param with same id
         if(!dest){dest=&New(); dest.id=base_param.id;}else if(dest.removed)dest.setRemoved(false);else return false; // if doesn't exist then create new and set id, if removed then un-remove, if exists then continue and don't override its value (this can happen if we're overriding value for multiple objects and some of which have this value already and some don't)
         SCAST(Param, *dest)=*base_param; dest.nameTypeValueUTC(); // set values and update times
         return true;
      }
      return false;
   }
   bool sync(C EditObject &src, cchar *edit_path)
   {
      bool changed=super.sync(src);
      changed|=Sync(       base_time, src.       base_time, base       , src.base       );
      changed|=Sync(editor_type_time, src.editor_type_time, editor_type, src.editor_type);
      if(Sync(access_time, src.access_time))
      {
         changed=true;
         access=src.access;
         FlagSet(flag, OVR_ACCESS, FlagTest(src.flag, OVR_ACCESS));
      }
      if(Sync(type_time, src.type_time))
      {
         changed=true;
         type=src.type;
         FlagSet(flag, OVR_TYPE, FlagTest(src.flag, OVR_TYPE));
      }
      if(Sync(const_time, src.const_time))
      {
         changed=true;
         FlagSet(flag, CONSTANT , src.constant());
         FlagSet(flag, OVR_CONST, FlagTest(src.flag, OVR_CONST));
      }
      if(Sync(path_time, src.path_time))
      {
         changed=true;
         path=src.path;
         FlagSet(flag, OVR_PATH, FlagTest(src.flag, OVR_PATH));
      }
      if(Sync(mesh_variation_time, src.mesh_variation_time))
      {
         changed=true;
         mesh_variation_id=src.mesh_variation_id;
         FlagSet(flag, OVR_MESH_VARIATION, FlagTest(src.flag, OVR_MESH_VARIATION));
      }
      REPA(src.sub_objs)
      {
       C SubObj &s=src.sub_objs[i];
         if(SubObj *dest=findSubObj(s.id))changed|=dest.sync(s);else{changed=true; sub_objs.add(s);}
      }
      if(changed)updateBase(edit_path);
      return changed;
   }
   bool undo(C EditObject &src, cchar *edit_path)
   {
      bool changed=super.undo(src);
      changed|=Undo(       base_time, src.       base_time, base       , src.base       );
      changed|=Undo(editor_type_time, src.editor_type_time, editor_type, src.editor_type);
      if(Undo(access_time, src.access_time))
      {
         changed=true;
         access=src.access;
         FlagSet(flag, OVR_ACCESS, FlagTest(src.flag, OVR_ACCESS));
      }
      if(Undo(type_time, src.type_time))
      {
         changed=true;
         type=src.type;
         FlagSet(flag, OVR_TYPE, FlagTest(src.flag, OVR_TYPE));
      }
      if(Undo(const_time, src.const_time))
      {
         changed=true;
         FlagSet(flag, CONSTANT , src.constant());
         FlagSet(flag, OVR_CONST, FlagTest(src.flag, OVR_CONST));
      }
      if(Undo(path_time, src.path_time))
      {
         changed=true;
         path=src.path;
         FlagSet(flag, OVR_PATH, FlagTest(src.flag, OVR_PATH));
      }
      if(Undo(mesh_variation_time, src.mesh_variation_time))
      {
         changed=true;
         mesh_variation_id=src.mesh_variation_id;
         FlagSet(flag, OVR_MESH_VARIATION, FlagTest(src.flag, OVR_MESH_VARIATION));
      }
      // sub objects
         // mark as removed those that aren't present in 'src'
         REPA(sub_objs)
         {
            SubObj &s=sub_objs[i]; if(!s.removed && !src.findSubObj(s.id)){s.removed=true; s.removed_time++; changed=true;}
         }
         // undo/insert from 'src'
         REPA(src.sub_objs)
         {
          C SubObj &s=src.sub_objs[i];
            SubObj *t=findSubObj(s.id);
            if(t)changed|=t.undo(s);else{sub_objs.add(s); changed=true;}
         }

      if(changed)updateBase(edit_path);
      return changed;
   }
   bool modify(C MemPtr<Edit.ObjChange> &changes, Project &proj)
   {
      bool  changed=false;
      FREPA(changes) // process in order
      {
       C Edit.ObjChange &change=changes[i];
         switch(change.cmd)
         {
            case Edit.EI_OBJ_PARAM_REMOVE_ID:
            {
               if(EditParam *param=EditParams.findParam(change.id))
                  if(!param.removed){changed=true; param.setRemoved(true);}
            }break;

            case Edit.EI_OBJ_PARAM_REMOVE_NAME:
            {
               if(EditParam *param=(InRange(change.type, PARAM_NUM) ? findParamInclRemoved(change.name, change.type) : findParamInclRemoved(change.name)))
                  if(!param.removed){changed=true; param.setRemoved(true);}
            }break;

            case Edit.EI_OBJ_PARAM_RESTORE_ID:
            {
               if(EditParam *param=EditParams.findParam(change.id))
                  if(param.removed){changed=true; param.setRemoved(false);}
            }break;

            case Edit.EI_OBJ_PARAM_RESTORE_NAME:
            {
               if(EditParam *param=(InRange(change.type, PARAM_NUM) ? findParamInclRemoved(change.name, change.type) : findParamInclRemoved(change.name)))
                  if(param.removed){changed=true; param.setRemoved(false);}
            }break;

            case Edit.EI_OBJ_PARAM_RENAME_ID:
            {
               if(EditParam *param=EditParams.findParam(change.id))
                  if(!Equal(param.name, change.param.name, true)){changed=true; param.setName(change.param.name);}
            }break;

            case Edit.EI_OBJ_PARAM_RENAME_NAME:
            {
               if(EditParam *param=(InRange(change.type, PARAM_NUM) ? findParamInclRemoved(change.name, change.type) : findParamInclRemoved(change.name)))
                  if(!Equal(param.name, change.param.name, true)){changed=true; param.setName(change.param.name);}
            }break;

            case Edit.EI_OBJ_PARAM_SET_TYPE_VALUE_ID:
            {
               if(EditParam *param=EditParams.findParam(change.id))
               {
                  if(CompareValue(*param, change.param)){changed=true; param.setTypeValue(change.param);} // set type value
                  if(param.removed){changed=true; param.setRemoved(false);} // restore
               }
            }break;

            case Edit.EI_OBJ_PARAM_SET_TYPE_VALUE_NAME:
            {
               if(EditParam *param=(InRange(change.type, PARAM_NUM) ? findParamInclRemoved(change.name, change.type) : findParamInclRemoved(change.name)))
               {
                  if(CompareValue(*param, change.param)){changed=true; param.setTypeValue(change.param);} // set type value
                  if(param.removed){changed=true; param.setRemoved(false);} // restore
               }else // create new one
               {
                  changed=true;
                  PARAM_TYPE type=(InRange(change.type, PARAM_NUM) ? change.type : change.param.type); // if type is not specified then use target value type
                  newParam(change.name, type, proj);
                  if(EditParam *param=(InRange(change.type, PARAM_NUM) ? findParamInclRemoved(change.name, change.type) : findParamInclRemoved(change.name)))
                  {
                     param.setTypeValue(change.param); // set type value
                     if(param.removed)param.setRemoved(false); // restore
                  }else // create new one
                  {
                     New().create(change.param).name=change.name;
                  }
               }
            }break;
         }
      }
      return changed;
   }

   // io
   bool save(File &f, cchar *edit_path=null)C
   {
      f.cmpUIntV(2);
      super.save(f, EditToGamePath(edit_path)); // 'EditParams' requires game path
      f<<flag<<base.id()<<editor_type;
      if(flag&OVR_ACCESS        )f<<access;
      if(flag&OVR_TYPE          )f<<type;
      if(flag&OVR_PATH          )f<<path;
      if(flag&OVR_MESH_VARIATION)f<<mesh_variation_id;
      f<<base_time<<access_time<<type_time<<editor_type_time<<const_time<<path_time<<mesh_variation_time;
      Save(f, sub_objs);
      return f.ok();
   }
   bool load(File &f, cchar *edit_path=null)
   {
      switch(f.decUIntV())
      {
         case 2:
         {
            if(!super.load(f, EditToGamePath(edit_path)))goto error; // 'EditParams' requires game path
            UID base_id;
            f>>flag>>base_id>>editor_type; if(base_id.valid())base=Str(edit_path).tailSlash(true)+EncodeFileName(base_id);else base=null;
            if(  flag&OVR_ACCESS        )f>>access           ;else access           =(base ? base->access            : OBJ_ACCESS_TERRAIN);
            if(  flag&OVR_TYPE          )f>>type             ;else if(base)type=base->type;else type.zero();
            if(!(flag&OVR_CONST)        )                     FlagSet(flag, CONSTANT, base ? base->constant()        : false             );
            if(  flag&OVR_PATH          )f>>path             ;else path             =(base ? base->path              : OBJ_PATH_CREATE   );
            if(  flag&OVR_MESH_VARIATION)f>>mesh_variation_id;else mesh_variation_id=(base ? base->mesh_variation_id : 0                 );
            f>>base_time>>access_time>>type_time>>editor_type_time>>const_time>>path_time>>mesh_variation_time;
            updateEditorType(edit_path);
            if(!Load(f, sub_objs))goto error;
            if(f.ok())return true;
         }break;

         case 1:
         {
            if(!super.load(f, EditToGamePath(edit_path)))goto error; // 'EditParams' requires game path
            UID base_id;
            f>>flag>>base_id>>editor_type; if(base_id.valid())base=Str(edit_path).tailSlash(true)+EncodeFileName(base_id);else base=null;
            if(  flag&OVR_ACCESS)f>>access;else access=(base ? base->access : OBJ_ACCESS_TERRAIN);
            if(  flag&OVR_TYPE  )f>>type  ;else if(base)type=base->type;else type.zero();
            if(!(flag&OVR_CONST))FlagSet(flag, CONSTANT, base ? base->constant() : false);
            if(  flag&OVR_PATH  )f>>path  ;else path=(base ? base->path : OBJ_PATH_CREATE);
            f>>base_time>>access_time>>type_time>>editor_type_time>>const_time>>path_time;
            updateEditorType(edit_path);
            if(!Load(f, sub_objs))goto error;
            mesh_variation_id=0; mesh_variation_time=0;
            if(f.ok())return true;
         }break;

         case 0:
         {
            if(!super.load(f, EditToGamePath(edit_path)))goto error; // 'EditParams' requires game path
            UID base_id;
            f>>flag>>base_id>>editor_type; if(base_id.valid())base=Str(edit_path).tailSlash(true)+EncodeFileName(base_id);else base=null;
            if(  flag&OVR_ACCESS)f>>access;else access=(base ? base->access : OBJ_ACCESS_TERRAIN);
            if(  flag&OVR_TYPE  )f>>type  ;else if(base)type=base->type;else type.zero();
            if(!(flag&OVR_CONST))FlagSet(flag, CONSTANT, base ? base->constant() : false);
            if(  flag&OVR_PATH  )f>>path  ;else path=(base ? base->path : OBJ_PATH_CREATE);
            f>>base_time>>access_time>>type_time>>editor_type_time>>const_time>>path_time;
            updateEditorType(edit_path);
            sub_objs.clear();
            mesh_variation_id=0; mesh_variation_time=0;
            if(f.ok())return true;
         }break;
      }
   error:
      del(); return false;
   }
   bool load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f, GetPath(name));
      del(); return false;
   }
}
Cache<EditObject> EditObjects;
typedef CacheElmPtr<EditObject, EditObjects> EditObjectPtr;
/******************************************************************************/
class GuiEditParam : EditParam
{
   class NameVal
   {
      cchar *name;
      int    val;
   }

   int  priority=0;
   Str _desc;
   bool min_use  =false, max_use  =false;
   dbl  min_value=0    , max_value=0    ;
   flt                  mouse_edit_speed=0; // 0=default
   PROP_MOUSE_EDIT_MODE mouse_edit_mode =PROP_MOUSE_EDIT_LINEAR;
   cchar8              *enum_name=null;
   Mems<NameVal>        name_vals;

 C Str & desc()C {return _desc;}   GuiEditParam& desc (C Str &desc     ) {_desc=desc; return T;}
                                   GuiEditParam& min  (dbl    min      ) {min_use=true; min_value=min; return T;}
                                   GuiEditParam& max  (dbl    max      ) {max_use=true; max_value=max; return T;}
                                   GuiEditParam& range(dbl min, dbl max) {return T.min(min).max(max);}

   GuiEditParam& mouseEditSpeed(flt                  speed) {mouse_edit_speed=speed; return T;}
   GuiEditParam& mouseEditMode (PROP_MOUSE_EDIT_MODE mode ) {mouse_edit_mode =mode ; return T;}
}
/******************************************************************************/
