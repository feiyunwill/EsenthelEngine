/******************************************************************************/
enum EDIT_OBJ_TYPE : byte
{
   EDIT_OBJ_MESH       ,
   EDIT_OBJ_LIGHT_POINT,
   EDIT_OBJ_LIGHT_CONE ,
   EDIT_OBJ_PARTICLES  ,
   EDIT_OBJ_NUM        ,
}
class EditObjTypeClass : Enum
{
   Memc<Memc<GuiEditParam>> params;

   EditObjTypeClass()
   {
      Enum.Elm elms[]=
      {
         {"EDIT_OBJ_MESH"       , UID(0, 0, 0, 0)}, // must be zeros because this is the default mode
         {"EDIT_OBJ_LIGHT_POINT", UID(1, 0, 0, 0)},
         {"EDIT_OBJ_LIGHT_CONE" , UID(2, 0, 0, 0)},
         {"EDIT_OBJ_PARTICLES"  , UID(3, 0, 0, 0)},
      };
      create("EDIT_OBJ_TYPE", elms);
      {
         Memc<GuiEditParam> &ps=params(EDIT_OBJ_LIGHT_POINT); int pr=-1024;
         {GuiEditParam &p=ps.New(); p.name="Color"       ; p.type=PARAM_COLOR; p.value.c=WHITE; p.id.set(1, 0, 0, 0); p.priority=pr++;}
         {GuiEditParam &p=ps.New(); p.name="Cast Shadows"; p.type=PARAM_BOOL ; p.value.b=true ; p.id.set(2, 0, 0, 0); p.priority=pr++;}
      }
      {
         Memc<GuiEditParam> &ps=params(EDIT_OBJ_LIGHT_CONE); int pr=-1024;
         {GuiEditParam &p=ps.New(); p.name="Color"       ; p.type=PARAM_COLOR; p.value.c=WHITE; p.id.set(1, 0, 0, 0); p.priority=pr++;}
         {GuiEditParam &p=ps.New(); p.name="Cast Shadows"; p.type=PARAM_BOOL ; p.value.b=true ; p.id.set(2, 0, 0, 0); p.priority=pr++;}
         {GuiEditParam &p=ps.New(); p.name="Angle"       ; p.type=PARAM_FLT  ; p.value.f=1    ; p.id.set(3, 0, 0, 0); p.priority=pr++;}
         {GuiEditParam &p=ps.New(); p.name="Falloff"     ; p.type=PARAM_FLT  ; p.value.f=0.5  ; p.id.set(4, 0, 0, 0); p.priority=pr++;}
      }
      {
         Memc<GuiEditParam> &ps=params(EDIT_OBJ_PARTICLES); int pr=-1024;
         {GuiEditParam &p=ps.New(); p.name="Image"             ; p.type=PARAM_ID   ; p.value.id.zero()          ; p.id.set( 5, 0, 0, 0); p.priority=pr++; p.desc("Image for the particles");}
         {GuiEditParam &p=ps.New(); p.name="Image Frames X"    ; p.type=PARAM_INT  ; p.value.i =1               ; p.id.set( 6, 0, 0, 0); p.priority=pr++; p.min(1).mouseEditSpeed(3).desc("Number of animation frames in image width, 1..65535, default=1");}
         {GuiEditParam &p=ps.New(); p.name="Image Frames Y"    ; p.type=PARAM_INT  ; p.value.i =1               ; p.id.set( 7, 0, 0, 0); p.priority=pr++; p.min(1).mouseEditSpeed(3).desc("Number of animation frames in image height, 1..65535, default=1");}
         {GuiEditParam &p=ps.New(); p.name="Image Speed"       ; p.type=PARAM_FLT  ; p.value.f =1.0             ; p.id.set( 8, 0, 0, 0); p.priority=pr++; p.min(0).desc("Image animation speed, 0..Inf, default=1");}
         {GuiEditParam &p=ps.New(); p.name="Palette Image"     ; p.type=PARAM_ID   ; p.value.id.zero()          ; p.id.set(34, 0, 0, 0); p.priority=pr++; p.desc("If adjust particle colors according to a custom Palette Image.\nParticle color will be multiplied by Image color.\nX coordinate of the image is based on particle life.\nY coordinate is assigned randomly at the particle birth.\nPalette Image must be in Software (IMAGE_SOFT) mode for this option to work.");}
         {GuiEditParam &p=ps.New(); p.name="Reborn"            ; p.type=PARAM_BOOL ; p.value.b =true            ; p.id.set( 9, 0, 0, 0); p.priority=pr++; p.desc("If particles should reborn after the end of their life");}
         {GuiEditParam &p=ps.New(); p.name="Emitter Life"      ; p.type=PARAM_FLT  ; p.value.f =0               ; p.id.set(35, 0, 0, 0); p.priority=pr++; p.desc("This specifies the life of the Particles Emitter\nValue <=0 specifies infinite life\nValue >0 specifies finite life");}
         {GuiEditParam &p=ps.New(); p.name="Fade In Time"      ; p.type=PARAM_FLT  ; p.value.f =1               ; p.id.set(36, 0, 0, 0); p.priority=pr++; p.desc("Time used to Fade In the Particle Emitter.\nThis is used only if the Emitter has finite life.");}
         {GuiEditParam &p=ps.New(); p.name="Fade Out Time"     ; p.type=PARAM_FLT  ; p.value.f =1               ; p.id.set(37, 0, 0, 0); p.priority=pr++; p.desc("Time used to Fade Out the Particle Emitter.\nThis is used only if the Emitter has finite life.");}
         {GuiEditParam &p=ps.New(); p.name="Palette Mode"      ; p.type=PARAM_BOOL ; p.value.b =true            ; p.id.set(10, 0, 0, 0); p.priority=pr++; p.desc("If draw particles using Palette or Blend mode\nParticles in Palette mode use only Alpha component from the Texture adjusted by global Renderer.color_palette texture\nParticles in Blend mode use all RGBA channels from the texture, they also need to have alpha color value enabled to make them visible");}
         {GuiEditParam &p=ps.New(); p.name="Palette Index"     ; p.type=PARAM_BOOL ; p.value.b =false           ; p.id.set(11, 0, 0, 0); p.priority=pr++; p.desc("Used when Palette Mode is enabled\nThis specifies global palette texture index:\nFalse: Renderer.color_palette using RM_PALETTE (default)\nTrue: Renderer.color_palette1 using RM_PALETTE1");}
         {GuiEditParam &p=ps.New(); p.name="Smooth Fade"       ; p.type=PARAM_BOOL ; p.value.b =false           ; p.id.set(12, 0, 0, 0); p.priority=pr++; p.desc("Specifies particle opacity depending on their age\nFalse: fade-in quickly, fade-out slowly (default)\nTrue: fade-in and fade-out smoothly");}
         {GuiEditParam &p=ps.New(); p.name="Elements"          ; p.type=PARAM_INT  ; p.value.i =40              ; p.id.set(13, 0, 0, 0); p.priority=pr++; p.range(0, 65535).desc("Number of particles");}
         {GuiEditParam &p=ps.New(); p.name="Color"             ; p.type=PARAM_COLOR; p.value.c.set(158, 0, 0, 0); p.id.set( 1, 0, 0, 0); p.priority=pr++; p.desc("Specifies:\n-Color for Particles in Blend mode\n-Palette Intensities for Particles in Palette mode");}
         {GuiEditParam &p=ps.New(); p.name="Glow"              ; p.type=PARAM_INT  ; p.value.i =0               ; p.id.set(14, 0, 0, 0); p.priority=pr++; p.range(0, 255).desc("Glow amount\nThis is used only for Particles in Blend mode");}
         {GuiEditParam &p=ps.New(); p.name="Radius"            ; p.type=PARAM_FLT  ; p.value.f =0.048           ; p.id.set(15, 0, 0, 0); p.priority=pr++; p.min(0).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).desc("Particle radius at its creation time, 0..Inf\nTthis is the initial value of 'Particle.radius' at its creation time.");}
         {GuiEditParam &p=ps.New(); p.name="Radius Random"     ; p.type=PARAM_FLT  ; p.value.f =0               ; p.id.set(32, 0, 0, 0); p.priority=pr++; p.min(0).                                      desc("Particle radius random factor, 0..Inf, default=0\nThis affects initial value of 'Particle.radius'\n0 value keeps it constant\n1 value can make it 2x smaller to 2x bigger\n2 value can make it 3x smaller to 3x bigger");}
         {GuiEditParam &p=ps.New(); p.name="Radius Growth"     ; p.type=PARAM_FLT  ; p.value.f =1.0             ; p.id.set(16, 0, 0, 0); p.priority=pr++; p.min(0).mouseEditMode(PROP_MOUSE_EDIT_SCALAR).desc("Particle radius growth factor, 0..Inf, default=1\nThis affects 'Particle.radius' in each frame, where a value of:\n0..1 - decreases the radius\n1 - keeps it constant\n1..Inf - increases the radius");}
         {GuiEditParam &p=ps.New(); p.name="Offset Range"      ; p.type=PARAM_FLT  ; p.value.f =0.0             ; p.id.set(17, 0, 0, 0); p.priority=pr++; p.min(0).desc("Particle position offset range applied during drawing, 0..Inf, default=0\nThis simulates the range of random position offsets for each particle during drawing\nvalue of 0 is the fastest");}
         {GuiEditParam &p=ps.New(); p.name="Offset Speed"      ; p.type=PARAM_FLT  ; p.value.f =1.0             ; p.id.set(18, 0, 0, 0); p.priority=pr++; p.min(0).desc("Particle position offset speed applied during drawing, 0..Inf, default=1\nThis simulates the speed of random potision offsets for each particle during drawing");}
         {GuiEditParam &p=ps.New(); p.name="Life"              ; p.type=PARAM_FLT  ; p.value.f =1.31            ; p.id.set(19, 0, 0, 0); p.priority=pr++; p.min(0).desc("Particle life, 0..Inf\nTthis is the life of a single particle 'Particle.max_life'");}
         {GuiEditParam &p=ps.New(); p.name="Life Random"       ; p.type=PARAM_FLT  ; p.value.f =0               ; p.id.set(33, 0, 0, 0); p.priority=pr++; p.min(0).desc("Particle life random factor, 0..Inf\nTthis affects 'Particle.max_life'\n0 value keeps it constant\n1 value can make it 2x shorter to 2x longer\n2 value can make it 3x shorter to 3x longer");}
         {GuiEditParam &p=ps.New(); p.name="Glue"              ; p.type=PARAM_FLT  ; p.value.f =0.0             ; p.id.set(20, 0, 0, 0); p.priority=pr++; p.range(0, 1).desc("Moves particles along with matrix movement, 0..1, default=0\nThis affects each single particle position according to source generation object movement, value of 0 disables position adjustment and doesn't require any additional calculations");}
         {GuiEditParam &p=ps.New(); p.name="Velocity Damping"  ; p.type=PARAM_FLT  ; p.value.f =0.0             ; p.id.set(21, 0, 0, 0); p.priority=pr++; p.range(0, 1).desc("Particle velocity damping, 0..1, default=0\nThis dampens particles velocity in each frame");}
         {GuiEditParam &p=ps.New(); p.name="Angular Velocity"  ; p.type=PARAM_FLT  ; p.value.f =0.0             ; p.id.set(22, 0, 0, 0); p.priority=pr++; p.min  (0   ).desc("Determines the 'Particle.ang_vel' to simulate particle rotations, 0..Inf, default=0");}
         {GuiEditParam &p=ps.New(); p.name="Random Velocity"   ; p.type=PARAM_FLT  ; p.value.f =0.084           ; p.id.set(23, 0, 0, 0); p.priority=pr++; p.min  (0   ).desc("Length of initial velocity applied at a random direction at particle creation, 0..Inf, default=0");}
         {GuiEditParam &p=ps.New(); p.name="Initial Velocity X"; p.type=PARAM_FLT  ; p.value.f =0.0             ; p.id.set(24, 0, 0, 0); p.priority=pr++; p.desc("Initial velocity applied at particle creation");}
         {GuiEditParam &p=ps.New(); p.name="Initial Velocity Y"; p.type=PARAM_FLT  ; p.value.f =0.0             ; p.id.set(25, 0, 0, 0); p.priority=pr++; p.desc("Initial velocity applied at particle creation");}
         {GuiEditParam &p=ps.New(); p.name="Initial Velocity Z"; p.type=PARAM_FLT  ; p.value.f =0.0             ; p.id.set(26, 0, 0, 0); p.priority=pr++; p.desc("Initial velocity applied at particle creation");}
         {GuiEditParam &p=ps.New(); p.name="Acceleration X"    ; p.type=PARAM_FLT  ; p.value.f =0.0             ; p.id.set(27, 0, 0, 0); p.priority=pr++; p.desc("Acceleration applied each frame");}
         {GuiEditParam &p=ps.New(); p.name="Acceleration Y"    ; p.type=PARAM_FLT  ; p.value.f =0.0             ; p.id.set(28, 0, 0, 0); p.priority=pr++; p.desc("Acceleration applied each frame");}
         {GuiEditParam &p=ps.New(); p.name="Acceleration Z"    ; p.type=PARAM_FLT  ; p.value.f =0.0             ; p.id.set(29, 0, 0, 0); p.priority=pr++; p.desc("Acceleration applied each frame");}
         {GuiEditParam &p=ps.New(); p.name="Source Shape"      ; p.type=PARAM_INT  ; p.value.i =SHAPE_BALL      ; p.id.set(30, 0, 0, 0); p.priority=pr++; p.desc("Shape type from which particles are generated");
            GuiEditParam.NameVal nvs[]=
            {
               {ShapeTypeName(SHAPE_POINT  ), SHAPE_POINT  },
               {ShapeTypeName(SHAPE_EDGE   ), SHAPE_EDGE   },
               {ShapeTypeName(SHAPE_RECT   ), SHAPE_RECT   },
               {ShapeTypeName(SHAPE_BOX    ), SHAPE_BOX    },
               {ShapeTypeName(SHAPE_CIRCLE ), SHAPE_CIRCLE },
               {ShapeTypeName(SHAPE_BALL   ), SHAPE_BALL   },
               {ShapeTypeName(SHAPE_CAPSULE), SHAPE_CAPSULE},
               {ShapeTypeName(SHAPE_TUBE   ), SHAPE_TUBE   },
               {ShapeTypeName(SHAPE_TORUS  ), SHAPE_TORUS  },
               {ShapeTypeName(SHAPE_CONE   ), SHAPE_CONE   },
               {ShapeTypeName(SHAPE_PYRAMID), SHAPE_PYRAMID},
            };
            p.enum_name="SHAPE_TYPE"; p.name_vals.setNum(Elms(nvs)); REPA(p.name_vals)Swap(p.name_vals[i], nvs[i]);
         }
         {GuiEditParam &p=ps.New(); p.name="Inside Source"      ; p.type=PARAM_BOOL ; p.value.b =true            ; p.id.set(31, 0, 0, 0); p.priority=pr++; p.desc("If generate particles inside shape or on its surface");}
         // !! BE CAREFUL WHEN ADDING NEW ID'S !!
      }
   }
 C Memc<GuiEditParam>* findParams(C UID &type_id)
   {
      int type=find(type_id);
      return InRange(type, params) ? &params[type] : null;
   }
}
EditObjTypeClass EditObjType;
/******************************************************************************/
class TerrainObj // Terrain Object for Embed Generation
{
   EditObjectPtr obj; // object

   operator bool()C {return obj!=null;}

   bool operator==(C TerrainObj &to)C {return obj==to.obj;}
   bool operator!=(C TerrainObj &to)C {return obj!=to.obj;}

   void set      (OBJ_ACCESS access, C EditObjectPtr &obj) {if(access==OBJ_ACCESS_TERRAIN && obj)T.obj=obj;else T.obj=null;}
   void operator=(C ObjData &obj                         ) {if(obj.removed)T.obj=null;else set(obj.params.access, obj.params.base);}

   TerrainObj(OBJ_ACCESS access, C EditObjectPtr &obj) {set(access, obj);}
   TerrainObj(C ObjData &obj                         ) {T=obj;}
}
class TerrainObj2 // Terrain Object for Embed Generation
{
   MeshPtr     mesh;
   PhysBodyPtr phys;

   operator bool()C {return mesh || phys;}

   bool operator==(C TerrainObj2 &to)C {return mesh==to.mesh && phys==to.phys;}
   bool operator!=(C TerrainObj2 &to)C {return mesh!=to.mesh || phys!=to.phys;}

   void operator=(C Object &obj) {if(obj.access()==OBJ_ACCESS_TERRAIN){T.mesh=obj.mesh(); T.phys=obj.phys();}else{T.mesh=null; T.phys=null;}}

   TerrainObj2() {}
   TerrainObj2(C Object &obj) {T=obj;}
}
/******************************************************************************/
class PhysPath // Phys Body Settings for Path Generation
{
   OBJ_PATH    mode=OBJ_PATH_IGNORE;
   PhysBodyPtr body;

   operator bool()C {return mode!=OBJ_PATH_IGNORE;}

   bool operator==(C PhysPath &pp)C {return mode==pp.mode && body==pp.body;}
   bool operator!=(C PhysPath &pp)C {return mode!=pp.mode || body!=pp.body;}

   void set      (OBJ_PATH mode, C PhysBodyPtr &body) {if(mode!=OBJ_PATH_IGNORE && body){T.mode=mode; T.body=body;}else{T.mode=OBJ_PATH_IGNORE; T.body=null;}}
   void operator=(C Object  &obj                    ) {set(obj.path(), obj.phys());}
   void operator=(C ObjData &obj                    )
   {
      if(!obj.removed && obj.params.path!=OBJ_PATH_IGNORE && obj.params.base)
      {
         Str game_base=EditToGamePath(obj.params.base.name());
         if( game_base.is()){T=*ObjectPtr(game_base); return;}
      }
      set(OBJ_PATH_IGNORE, null); // set empty
   }

   PhysPath(OBJ_PATH mode, C PhysBodyPtr &body) {set(mode, body);}
   PhysPath(C Object  &obj                    ) {T=obj;}
   PhysPath(C ObjData &obj                    ) {T=obj;}
}
/******************************************************************************/
class ObjData // World Object
{
   bool       removed=false;
   UID        id=UID().randomizeValid(); // Instance ID
   Matrix     matrix(1);
   EditObject params;
   TimeStamp  matrix_time, removed_time;

   // get
   bool old(C TimeStamp &now=TimeStamp().getUTC())C {return matrix_time<now && removed_time<now && params.old();}

   bool equal(C ObjData &src)C {return matrix_time==src.matrix_time && removed_time==src.removed_time && params.equal(src.params);}
   bool newer(C ObjData &src)C {return matrix_time> src.matrix_time || removed_time> src.removed_time || params.newer(src.params);}

   TerrainObj terrainObj()C {return T;}
   PhysPath   physPath  ()C {return T;}

   uint memUsage()C {return params.memUsage();}

   // operations
   void setRemoved(bool removed, C TimeStamp &time=TimeStamp().getUTC()) {T.removed=removed; T.removed_time=time;}

   void create(C ObjData &src)
   {
      params.create(src.params);
      removed=src.removed; removed_time=src.removed_time;
      matrix =src.matrix ;  matrix_time=src. matrix_time;
   }
   bool sync(C ObjData &src, cchar *edit_path)
   {
      bool changed=params.sync(src.params, edit_path);
      changed|=Sync( matrix_time, src. matrix_time,  matrix, src. matrix);
      changed|=Sync(removed_time, src.removed_time, removed, src.removed);
      return changed;
   }
   bool undo(C ObjData &src, cchar *edit_path)
   {
      bool changed=params.undo(src.params, edit_path);
      changed|=Undo(removed_time, src.removed_time, removed, src.removed);
      changed|=Undo( matrix_time, src. matrix_time,  matrix, src. matrix);
      return changed;
   }
   void create(C Game.Area.Data.AreaObj &src, C UID &type, C EditObjectPtr &base, C TimeStamp &time=TimeStamp().getUTC())
   {
      removed=false;
      id=src.id;
      matrix=src.matrixFinal();
      params.create(src, type, base, time);
      matrix_time=removed_time=time;
   }
   void copyTo(Game.Area.Data.AreaObj &dest, C Project &proj)C
   {
      params.copyTo(dest, proj, false, null, null); // assumes that world object cannot override mesh/phys
      dest.scale(true, matrix.scale()); dest.matrix=matrix; // set custom scale
      dest.id=id;
   }
   ObjData& create(C Edit.WorldObjParams &src, C Str &edit_path)
   {
      params.setBase(EditObjectPtr(edit_path+EncodeFileName(src.id)), edit_path);
      matrix=src.matrix;
      removed_time=matrix_time.getUTC();
      return T;
   }

   // io
   bool save(File &f, cchar *edit_path=null)C
   {
      f.cmpUIntV(0);
      f<<id<<matrix<<removed<<matrix_time<<removed_time;
      if(params.save(f, edit_path))
         return f.ok();
      return false;
   }
   bool load(File &f, cchar *edit_path=null)
   {
      switch(f.decUIntV())
      {
         case 0:
         {
            f>>id>>matrix>>removed>>matrix_time>>removed_time;
            if(params.load(f, edit_path))
               if(f.ok())return true;
         }break;
      }
      return false;
   }
}
/******************************************************************************/
void Adjust(Particles &particles, C EditObject &obj, C Matrix &matrix, Project &proj) // adjust particles without resetting them because this can be called each frame when editing objects
{
   if(obj.editor_type==EditObjType.elmID(EDIT_OBJ_PARTICLES))
   {
      UID image=UIDZero; if(C EditParam *p=obj.findParam("Image"        ))if(Elm *elm=proj.findElm(p.asID()))if(ElmImageLike(elm.type))image=elm.id; particles.        image=proj.gamePath(image); // accept only images
          image=UIDZero; if(C EditParam *p=obj.findParam("Palette Image"))if(Elm *elm=proj.findElm(p.asID()))if(ElmImageLike(elm.type))image=elm.id; particles.palette_image=proj.gamePath(image); // accept only images
      if(C EditParam *p=obj.findParam("Image Frames X"    ))particles.image_x_frames  =Mid(p.asInt  (), 1, 65535);else particles.image_x_frames=1;
      if(C EditParam *p=obj.findParam("Image Frames Y"    ))particles.image_y_frames  =Mid(p.asInt  (), 1, 65535);else particles.image_y_frames=1;
      if(C EditParam *p=obj.findParam("Image Speed"       ))particles.image_speed     =    p.asFlt  ()           ;else particles.image_speed=1;
      if(C EditParam *p=obj.findParam("Reborn"            ))particles.reborn          =    p.asBool ()           ;else particles.reborn=true;
      if(C EditParam *p=obj.findParam("Emitter Life"      ))particles.emitter_life_max=Max(p.asFlt  (), 0       );else particles.emitter_life_max=0;
      if(C EditParam *p=obj.findParam("Fade In Time"      ))particles.fade_in         =Max(p.asFlt  (), 0       );else particles.fade_in =1;
      if(C EditParam *p=obj.findParam("Fade Out Time"     ))particles.fade_out        =Max(p.asFlt  (), 0       );else particles.fade_out=1;
      if(C EditParam *p=obj.findParam("Palette Mode"      ))particles.palette         (    p.asBool ()          );else particles.palette(true);
      if(C EditParam *p=obj.findParam("Palette Index"     ))particles.paletteIndex    (    p.asBool ()          );else particles.paletteIndex(false);
      if(C EditParam *p=obj.findParam("Smooth Fade"       ))particles.smooth_fade     =    p.asBool ()           ;else particles.smooth_fade=false;
      if(C EditParam *p=obj.findParam("Elements"          ))particles.p.setNumZero    (Max(p.asInt  (), 0      ));else particles.p.setNumZero(40);
      if(C EditParam *p=obj.findParam("Color"             ))particles.color           =    p.asColor()           ;else particles.color.set(158, 0, 0, 0);
      if(C EditParam *p=obj.findParam("Glow"              ))particles.glow            =Mid(p.asInt  (), 0, 255  );else particles.glow=0;
      if(C EditParam *p=obj.findParam("Radius"            ))particles.radius          =    p.asFlt  ()           ;else particles.radius=0.048;
      if(C EditParam *p=obj.findParam("Radius Random"     ))particles.radius_random   =Max(p.asFlt  (), 0       );else particles.radius_random=0;
      if(C EditParam *p=obj.findParam("Radius Growth"     ))particles.radius_growth   =Max(p.asFlt  (), 0       );else particles.radius_growth=1;
      if(C EditParam *p=obj.findParam("Offset Range"      ))particles.offset_range    =    p.asFlt  ()           ;else particles.offset_range=0;
      if(C EditParam *p=obj.findParam("Offset Speed"      ))particles.offset_speed    =    p.asFlt  ()           ;else particles.offset_speed=1;
      if(C EditParam *p=obj.findParam("Life"              ))particles.life            =Max(p.asFlt  (), 0       );else particles.life=1.31;
      if(C EditParam *p=obj.findParam("Life Random"       ))particles.life_random     =Max(p.asFlt  (), 0       );else particles.life_random=1.31;
      if(C EditParam *p=obj.findParam("Glue"              ))particles.glue            =Sat(p.asFlt  ()          );else particles.glue=0;
      if(C EditParam *p=obj.findParam("Velocity Damping"  ))particles.damping         =Sat(p.asFlt  ()          );else particles.damping=0;
      if(C EditParam *p=obj.findParam("Angular Velocity"  ))particles.ang_vel         =    p.asFlt  ()           ;else particles.ang_vel=0;
      if(C EditParam *p=obj.findParam("Random Velocity"   ))particles.vel_random      =    p.asFlt  ()           ;else particles.vel_random=0;
      if(C EditParam *p=obj.findParam("Initial Velocity X"))particles.vel_constant.x  =    p.asFlt  ()           ;else particles.vel_constant.x=0;
      if(C EditParam *p=obj.findParam("Initial Velocity Y"))particles.vel_constant.y  =    p.asFlt  ()           ;else particles.vel_constant.y=0;
      if(C EditParam *p=obj.findParam("Initial Velocity Z"))particles.vel_constant.z  =    p.asFlt  ()           ;else particles.vel_constant.z=0;
      if(C EditParam *p=obj.findParam("Acceleration X"    ))particles.accel.x         =    p.asFlt  ()           ;else particles.accel.x=0;
      if(C EditParam *p=obj.findParam("Acceleration Y"    ))particles.accel.y         =    p.asFlt  ()           ;else particles.accel.y=0;
      if(C EditParam *p=obj.findParam("Acceleration Z"    ))particles.accel.z         =    p.asFlt  ()           ;else particles.accel.z=0;
      if(C EditParam *p=obj.findParam("Inside Source"     ))particles.inside_shape    =    p.asBool ()           ;else particles.inside_shape=true;
      SHAPE_TYPE shape_type=SHAPE_NONE; if(C EditParam *p=obj.findParam("Source Shape"))shape_type=p.asEnum(SHAPE_NONE);
      switch(shape_type)
      {
         default           : particles.source(Ball(0.5)); break; // SHAPE_BALL
         case SHAPE_POINT  : particles.source(Vec(0)); break;
         case SHAPE_EDGE   : particles.source(Edge(Vec(-0.5, 0, 0), Vec(0.5, 0, 0))); break;
         case SHAPE_RECT   : particles.source(Rect(-0.5, 0.5)); break;
         case SHAPE_BOX    : particles.source(Box(0.5)); break;
         case SHAPE_OBOX   : particles.source(OBox(Box(0.5))); break;
         case SHAPE_CIRCLE : particles.source(Circle(0.5)); break;
         case SHAPE_CAPSULE: particles.source(Capsule(0.3, 1)); break;
         case SHAPE_TUBE   : particles.source(Tube(0.3, 1)); break;
         case SHAPE_TORUS  : particles.source(Torus(0.3, 0.1)); break;
         case SHAPE_CONE   : particles.source(Cone(0.5, 0, 1, Vec(0, -0.5, 0))); break;
         case SHAPE_PYRAMID: particles.source(Pyramid(0.5, 1, Vec(0, 0.5, 0), Vec(0, -1, 0))); break;
      }
      particles.matrix       =matrix; flt scale=matrix.avgScale();
      particles.radius      *=scale;
      particles.offset_range*=scale;
      particles.vel_random  *=scale;
      particles.vel_constant*=matrix.orn();
      particles.accel       *=matrix.orn();
   }else
   {
      particles.del();
   }
}
/******************************************************************************/
