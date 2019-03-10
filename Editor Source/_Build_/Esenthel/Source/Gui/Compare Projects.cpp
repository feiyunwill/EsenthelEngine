/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
CompareProjects CompareProjs;
/******************************************************************************/

/******************************************************************************/
      void CompareProjects::ElmListElm::setElm(C Project &proj, C Elm &elm)
      {
         T.name=proj.elmFullName(&elm);
         T.type=elm.type;
         T.type_name=ElmTypeName[elm.type];
         T.id=elm.id;
      }
   void CompareProjects::Changed(C Property &prop) {CompareProjs.refresh();}
   void CompareProjects::SelectSel(CompareProjects &cp)
   {
      Memt<UID> ids; FREPA(cp.list.sel)if(ElmListElm *elml=cp.list.absToData(cp.list.sel[i]))ids.add(elml->id); Proj.elmSelect(ids);
   }
   void CompareProjects::SelectAll(CompareProjects &cp)
   {
      Memt<UID> ids; FREPA(cp.data)ids.add(cp.data[i].id); Proj.elmSelect(ids);
   }
   void CompareProjects::release()
   {
      projects.clear();
      list.clear();
      data.clear();
   }
   CompareProjects& CompareProjects::del(){release(); ::EE::Window::del (); return T;}
   CompareProjects& CompareProjects::hide(){release(); ::PropWin::hide(); return T;}
   CompareProjects& CompareProjects::rect(C Rect &rect)
{
      ::EE::Window::rect(rect);
      flt p=0.02f;
      region.rect(Rect(p, -clientHeight()+p, clientWidth()-p, prop_min_y-p));
      return T;
   }
   void CompareProjects::create()
   {
      add("Detect Data Differences"   , MEMBER(CompareProjects, test_data));
      add("Detect Name Differences"   , MEMBER(CompareProjects, test_name));
      add("Detect Parent Differences" , MEMBER(CompareProjects, test_parent));
      add("Detect Publish Differences", MEMBER(CompareProjects, test_publish));
      add("Display New Elements"      , MEMBER(CompareProjects, display_new)).desc("This will display all elements that exist in one project, but don't exist in the other one.");
      Rect r=::PropWin::create("Project Comparison"); prop_min_y=r.min.y; autoData(this); ::PropWin::changed(Changed); flag|=WIN_RESIZABLE; button[1].show(); button[2].show();
      ListColumn lc[]=
      {
         ListColumn(MEMBER(ElmListElm,      name), 1.00f, "Name"), // 0
         ListColumn(MEMBER(ElmListElm, type_name), 0.22f, "Type"), // 1
      };
      T+=select_selected.create(Rect_L(                r     .right()+Vec2(0.05f, 0), 0.3f, 0.06f), "Select Picked").func(SelectSel, T).desc("This option will select the Project Elements which are picked on this list");
      T+=select_all     .create(Rect_L(select_selected.rect().right()+Vec2(0.05f, 0), 0.3f, 0.06f), "Select All"   ).func(SelectAll, T).desc("This option will select all Project Elements which are visible on this list");
      T+=region.create();
      region+=list.create(lc, Elms(lc)).elmHeight(0.038f).textSize(0, 1); list.flag|=LIST_RESIZABLE_COLUMNS|LIST_MULTI_SEL; list.cur_mode=LCM_ALWAYS;
      list.sort_column[0]=0;
      rect(Rect_C(0, 0, 1.22f+0.02f*2+region.slidebarSize(), 1.8f));
   }
   bool CompareProjects::DiffElm(C Project &proj_a, C Elm &elm_a, C Project &proj_b, C Elm &elm_b)
   {
      if(elm_a.newerData(elm_b) || elm_a.newerFile(elm_b)
      || elm_b.newerData(elm_a) || elm_b.newerFile(elm_a))return true;
      if(!FEqual(proj_a.editPath(elm_a), proj_b.editPath(elm_b))
      || !FEqual(proj_a.gamePath(elm_a), proj_b.gamePath(elm_b)))return true;
      return false;
   }
   void CompareProjects::refresh()
   {
      data.clear();
      if(projects.elms()>=2)
      {
         Memc<UID> different;
         FREPAD(pa, projects) // start from the beginning to list elements preferrably with pathnames from the first project
         {
          C Project &proj_a=projects[pa];
            FREPAD(ea, proj_a.elms) // start from the beginning to minimize 'binaryInclude' time
            {
             C Elm &elm_a=proj_a.elms[ea];
               if(elm_a.finalExists())
                  REPAD(pb, projects) // compare all projects (and not only those before 'proj_a', something like "REP(ea)" because we need to check all elements of both projects compared to others in case 'display_new' is selected)
                  if(pb!=pa) // don't compare the same project
               {
                C Project &proj_b=projects[pb];
                C Elm     * elm_b=proj_b.findElm(elm_a.id, elm_a.type);
                  if(elm_b && elm_b->finalExists())
                  {
                     if(test_data    && (elm_a.data ? elm_a.data->ver.ver : 0)!=(elm_b->data ? elm_b->data->ver.ver : 0) && DiffElm(proj_a, elm_a, proj_b, *elm_b)
                     || test_name    && elm_a.  name_time!=elm_b->  name_time && !Equal(elm_a.name, elm_b->name, true)
                     || test_parent  && elm_a.parent_time!=elm_b->parent_time &&        elm_a.parent_id!=elm_b->parent_id
                     || test_publish && elm_a.noPublish()!=elm_b->noPublish())
                        if(different.binaryInclude(elm_a.id, Compare))
                           data.New().setElm(proj_a, elm_a);
                  }else
                  if(display_new)
                     if(different.binaryInclude(elm_a.id, Compare))
                        data.New().setElm(proj_a, elm_a);
               }
            }
         }
      }
      list.setData(data);
   }
   void CompareProjects::changed(C UID &proj_id)
   {
      if(visible())REPA(projects)
      {
         Project &proj=projects[i]; if(proj.id==proj_id)
         {
            if(proj_id==Proj.id)proj=Proj;else
            {
               Str path=proj.path;
               int ver; Str error; proj.load3(path, ver, error);
               proj.setIDPath(proj_id, path);
            }
            refresh(); break;
         }
      }
   }
   void CompareProjects::compare(C MemPtr<UID> &proj_ids)
   {
      release(); hide();
      FREPA(proj_ids) // process in order
      {
       C Projects::Elm *proj=Projs.findProj(proj_ids[i]); if(!proj){Gui.msgBox(S, S+"Project \""+proj_ids[i].asFileName()+"\" was not found"); return;}
         if(!proj->path.is()){Gui.msgBox(S, S+"Project \""+proj->name+"\" has no path"); return;}
         Project temp; int ver; Str error; LOAD_RESULT result=temp.load3(proj->path, ver, error);
         if(LoadOK(result))
         {
            temp.setIDPath(proj->id, proj->path);
            Swap(projects.New(), temp);
         }else switch(result)
         {
            case LOAD_NEWER : Gui.msgBox(S, S+"Project \""+proj->name+"\" requires newer engine version."); return;
            case LOAD_ERROR : Gui.msgBox(S, S+"Project \""+proj->name+"\" failed to load."+(error.is() ? '\n' : '\0')+error); return;
            case LOAD_LOCKED: Gui.msgBox(S, S+"Project \""+proj->name+"\" is locked."); return;
         }
      }
      refresh();
      activate();
   }
   void CompareProjects::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(visible() && gpc.visible)
      {
         select_selected.visible(Proj.valid());
         select_all     .visible(Proj.valid());

         if(Ms.bd(       0) && Gui.ms()==&list
         || Kb.k (KB_ENTER) && Gui.kb()==&list)
            if(ElmListElm *list_elm=list())
               if(Elm *elm=Proj.findElm(list_elm->id))
         {
            if(!ElmVisible(elm->type))elm=Proj.firstVisibleParent(elm);
            if(elm)
            {
               if(elm->type==ELM_FOLDER)Proj.elmOpenParents(elm->id);
               else                    Proj.elmToggle     (elm->id); // call this first because it may open parents
               Proj.elmLocate(list_elm->id, true); // call this next once parents are opened
            }
         }
      }
   }
CompareProjects::CompareProjects() : test_data(true), test_name(true), test_parent(false), test_publish(false), display_new(false), prop_min_y(0) {}

CompareProjects::ElmListElm::ElmListElm() : type_name(null), id(UIDZero), type(ELM_NONE) {}

/******************************************************************************/
