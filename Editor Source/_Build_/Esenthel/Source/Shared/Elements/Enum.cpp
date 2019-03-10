/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   EditEnum& EditEnum::setName(C Str &name ) {T.name   =name   ;    name_time.getUTC(); return T;}
   EditEnum& EditEnum::setRemoved(bool removed) {T.removed=removed; removed_time.getUTC(); return T;}
   bool EditEnum::newer(C EditEnum &src)C
   {
      return removed_time>src.removed_time || name_time>src.name_time || order_time>src.order_time;
   }
   bool EditEnum::equal(C EditEnum &src)C
   {
      return removed_time==src.removed_time && name_time==src.name_time && order_time==src.order_time;
   }
   bool EditEnum::sync(C EditEnum &src)
   {
      bool changed=false;
      // 'order_time' is synced in 'EditEnums.sync'
      changed|=Sync(removed_time, src.removed_time, removed, src.removed);
      changed|=Sync(   name_time, src.   name_time,    name, src.   name);
      return changed;
   }
   bool EditEnum::undo(C EditEnum &src)
   {
      bool changed=false;
      changed|=Undo(removed_time, src.removed_time, removed, src.removed);
      changed|=Undo(   name_time, src.   name_time,    name, src.   name);
      changed|=Undo(  order_time, src.  order_time);
      return changed;
   }
   bool EditEnum::save(File &f)C
   {
      f.cmpUIntV(2);
      f<<id<<name<<removed<<removed_time<<name_time<<order_time;
      return f.ok();
   }
   bool EditEnum::load(File &f)
   {
      switch(f.decUIntV())
      {
         case 2:
         {
            f>>id>>name>>removed>>removed_time>>name_time>>order_time;
            if(f.ok())return true;
         }break;

         case 1:
         {
            f>>id; GetStr2(f, name); f>>removed>>removed_time>>name_time>>order_time;
            if(f.ok())return true;
         }break;

         case 0:
         {
            f>>id>>removed>>removed_time>>name_time>>order_time;
            GetStr(f, name);
            if(f.ok())return true;
         }break;
      }
      return false;
   }
   int       EditEnums::FindI(C Memc<EditEnum> &enums, C UID &enum_id) {REPA(enums)if(enums[i].id==enum_id)return i; return -1;}
   EditEnum* EditEnums::Find(  Memc<EditEnum> &enums, C UID &enum_id) {int i=FindI(enums, enum_id); return (i>=0) ? &enums[i] : null;}
   C EditEnum* EditEnums::Find(C Memc<EditEnum> &enums, C UID &enum_id) {return Find(ConstCast(enums), enum_id);}
   void EditEnums::del() {clear(); type=DEFAULT; type_time.zero();}
   int       EditEnums::findI(C UID &enum_id)C {return FindI(T, enum_id);}
   EditEnum* EditEnums::find(C UID &enum_id)  {return Find (T, enum_id);}
 C EditEnum* EditEnums::find(C UID &enum_id)C {return Find (T, enum_id);}
   bool EditEnums::newer(C EditEnums &src)C
   {
      REPA(T){C EditEnum &e=T[i], *s=src.find(e.id); if(!s || e.newer(*s))return true;}
      return type_time>src.type_time;
   }
   bool EditEnums::equal(C EditEnums &src)C
   {
      if(elms()!=src.elms())return false;
      REPA(T){C EditEnum &e=T[i], *s=src.find(e.id); if(!s || !e.equal(*s))return false;}
      return type_time==src.type_time;
   }
   bool EditEnums::sync(C EditEnums &src)
   {
      bool changed=false;
      Memc<EditEnum> this_enums, src_enums=src;
      Swap(SCAST(Memc<EditEnum>, T), this_enums);
      for(; this_enums.elms() || src_enums.elms(); )
      {
         EditEnum *t=this_enums.addr(0), // this elm
                  *s= src_enums.addr(0), // src  elm
                  *t_in_s=(t ? Find( src_enums, t->id) : null), // this in src
                  *s_in_t=(s ? Find(this_enums, s->id) : null); // src  in this
         if(t && t_in_s && t_in_s->order_time>t->order_time) // this elm is present in src and there it has newer order
         {
            changed=true; t_in_s->sync(*t); this_enums.removeData(t, true);
         }else
         if(s && s_in_t && s_in_t->order_time>s->order_time) // src elm is present in this and there it has newer order
         {
            changed=true; s_in_t->sync(*s); src_enums.removeData(s, true);
         }else
         if(t && (!s || t->order_time>=s->order_time)) // there is this elm which is same or newer than src elm
         {
            if(t_in_s){changed|=t->sync(*t_in_s); src_enums.removeData(t_in_s, true);}
            Swap(*t, New()); this_enums.removeData(t, true);
         }else
         if(s)
         {
            changed=true; // always changed because we're taking src element instead of this elm
            if(s_in_t){changed|=s->sync(*s_in_t); this_enums.removeData(s_in_t, true);}
            Swap(*s, New()); src_enums.removeData(s, true);
         }else Exit("EditEnums::sync");
      }
      changed|=Sync(type_time, src.type_time, type, src.type);
      return changed;
   }
   void EditEnums::undo(C EditEnums &src)
   {
      Memc<EditEnum> this_enums, src_enums=src;
      Swap(SCAST(Memc<EditEnum>, T), this_enums);
      for(; this_enums.elms() || src_enums.elms(); )
      {
         EditEnum *t=this_enums.addr(0), // this elm
                  *s= src_enums.addr(0), // src  elm
                  *t_in_s=(t ? Find( src_enums, t->id) : null), // this in src
                  *s_in_t=(s ? Find(this_enums, s->id) : null); // src  in this
         if(t && !t_in_s) // this elm is present and it is not found in src (which means that it was created later, so add it first as removed)
         {
            if(!t->removed){t->removed=true; t->removed_time++;} // mark as removed if necessary
            Swap(*t, New()); this_enums.removeData(t, true);
         }else
         if(s)
         {
            if(s_in_t) // 's_in_t' is newer, so undo and insert this one
            {
               s_in_t->undo(*s); Swap(*s_in_t, New());
            }else
            {
               Swap(*s, New());
            }
            this_enums.removeData(s_in_t, true);
             src_enums.removeData(s     , true);
         }else Exit("EditEnums::undo");
      }
      Undo(type_time, src.type_time, type, src.type);
   }
   int EditEnums::move(C UID &enum_id, int index)
   {
      int elm=FindI(T, enum_id);
      if(InRange(elm, T))
      {
         T[elm].order_time.getUTC();
         moveElm(elm, index);
         if(elm>=index)index++;
      }
      return index;
   }
   void EditEnums::copyTo(Enum &e, C Str &name)C
   {
      Memt<Enum::Elm> elms; FREPA(T)if(!T[i].removed)elms.New().set(T[i].name, T[i].id);
      e.create(NameToEnum(name), elms);
   }
   void EditEnums::create(C Enum &src)
   {
      del();
      FREPA(src)New().setName(src[i].name).id=src[i].id;
   }
   bool EditEnums::save(File &f)C
   {
      f.cmpUIntV(1);
      f<<type<<type_time;
      return ::EE::Memc< ::EditEnum>::save(f);
   }
   bool EditEnums::load(File &f)
   {
      switch(f.decUIntV())
      {
         case 1: f>>type>>type_time; return ::EE::Memc< ::EditEnum>::load(f);
         case 0: type=DEFAULT; type_time.zero(); return ::EE::Memc< ::EditEnum>::load(f);
      }
      del(); return false;
   }
   bool EditEnums::load(C Str &name)
   {
      File f; if(f.readTry(name))return load(f); del(); return false;
   }
EditEnum::EditEnum() : removed(false), id(UID().randomizeValid()) {}

EditEnums::EditEnums() : type(DEFAULT) {}

/******************************************************************************/
