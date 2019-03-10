/******************************************************************************/
/******************************************************************************/
class CompareProjects : PropWin
{
   class ElmListElm // project element as list element
   {
      cchar8  *type_name;
      Str      name;
      UID      id;
      ELM_TYPE type;

      void setElm(C Project &proj, C Elm &elm);

public:
   ElmListElm();
   };

   bool test_data, test_name, test_parent, test_publish, display_new;

   flt              prop_min_y;
   Button           select_selected, select_all;
   Memc<Project   > projects;
   Memc<ElmListElm> data;
   List<ElmListElm> list;
   Region           region;
 //ViewportSkin     preview;

 /*static void DrawPreview(Viewport &viewport) {SizeStats.drawPreview();}
          void drawPreview()
   {
      if(ElmListElm *list_elm=list())
      {
         if(list_elm.type==ELM_IMAGE) // for images and textures read them directly from the data
         {
            // load
            if(image_id!=list_elm.id)
            {
               image_id=list_elm.id;
               Str name=(list_elm.path.is() ? list_elm.path : path+EncodeFileName(image_id));
               File f; if(pak.totalFiles())f.readTry(name, pak);else f.readTry(name);
               image.load(f);
            }

            // draw
            ALPHA_MODE alpha=D.alpha();
            if(!list_elm.proj_elm)D.alpha(ALPHA_NONE); // disable alpha blending for textures
            Rect rect=image.fit(D.viewRect());
            if(image.mode()==IMAGE_CUBE)image.drawCubeFace(WHITE, TRANSPARENT, rect, DIR_FORWARD);
            else                        image.draw        (rect);
            D.alpha(alpha);
            TextStyleParams ts; ts.align.set(1, 1); D.text(ts, D.viewRect().ld(), S+image.w()+'x'+image.h()+' '+ImageTI[image.type()].name+", "+image.mipMaps()+" MipMap"+CountS(image.mipMaps()));
         }else // other elements preview as normal for simplification
         if(list_elm.proj_elm)if(Elm *elm=Proj.findElm(list_elm.id))Preview.draw(*elm, D.viewRect());
      }
   }*/

   static void Changed(C Property &prop);     
   static void SelectSel(CompareProjects &cp);
   static void SelectAll(CompareProjects &cp);

   void release();
   virtual CompareProjects& del ()override;
   virtual CompareProjects& hide()override;

   virtual CompareProjects& rect(C Rect &rect)override;
   void create();
   static bool DiffElm(C Project &proj_a, C Elm &elm_a, C Project &proj_b, C Elm &elm_b);
   void refresh();
   void changed(C UID &proj_id);
   void compare(C MemPtr<UID> &proj_ids);
   virtual void update(C GuiPC &gpc)override;

public:
   CompareProjects();
};
/******************************************************************************/
/******************************************************************************/
extern CompareProjects CompareProjs;
/******************************************************************************/
