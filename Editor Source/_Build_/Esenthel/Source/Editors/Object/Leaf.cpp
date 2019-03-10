/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/

/******************************************************************************/
   void LeafRegion::RemoveAttachment(LeafRegion &leaf) {ObjEdit.remVtx(VTX_HLP  , MtrlEdit.game, true);}
   void LeafRegion::RemoveBending(LeafRegion &leaf) {ObjEdit.remVtx(VTX_SIZE , MtrlEdit.game, true);}
   void LeafRegion::RemoveColor(LeafRegion &leaf) {ObjEdit.remVtx(VTX_COLOR, MtrlEdit.game, true);}
   void LeafRegion::SetAttachmentCam(LeafRegion &leaf)
   {
      bool changed=false;
      Vec  pos=ObjEdit.v4.last()->camera.at/ObjEdit.mesh_matrix;
      REPD(i, ObjEdit.mesh.lods())if(ObjEdit.selLod()==i || !ObjEdit.lod_tabs.visibleFull())
      {
         MeshLod &lod=ObjEdit.mesh.lod(i); REPA(lod.parts)if(ObjEdit.partOp(i) || !ObjEdit.mesh_parts.visibleFull())
         {
            MeshPart &part=lod.parts[i]; if(HasMaterial(part, MtrlEdit.game)){ObjEdit.mesh_undos.set("leaf"); part.setLeafAttachment(pos); changed=true;}
         }
      }
      if(changed)ObjEdit.setChangedMesh(true, false);
   }
   void LeafRegion::RandomBending(LeafRegion &leaf)
   {
      ObjEdit.mesh_undos.set("bending");
      bool changed=false;
      REPD(i, ObjEdit.mesh.lods())if(ObjEdit.selLod()==i || !ObjEdit.lod_tabs.visibleFull())
      {
         MeshLod &lod=ObjEdit.mesh.lod(i); REPA(lod.parts)if(ObjEdit.partOp(i) || !ObjEdit.mesh_parts.visibleFull())
         {
            MeshPart &part=lod.parts[i]; if(HasMaterial(part, MtrlEdit.game) && (part.flag()&VTX_HLP)) // set only for parts that already have leaf attachment
            {
               part.setRandomLeafBending(); changed=true;
            }
         }
      }
      if(changed)ObjEdit.setChangedMesh(true, false);
   }
   void LeafRegion::SameRandomBending(LeafRegion &leaf)
   {
      ObjEdit.mesh_undos.set("bending");
      bool changed=false;
      flt  random=Random.f(1024);
      REPD(i, ObjEdit.mesh.lods())if(ObjEdit.selLod()==i || !ObjEdit.lod_tabs.visibleFull())
      {
         MeshLod &lod=ObjEdit.mesh.lod(i); REPA(lod.parts)if(ObjEdit.partOp(i) || !ObjEdit.mesh_parts.visibleFull())
         {
            MeshPart &part=lod.parts[i]; if(HasMaterial(part, MtrlEdit.game) && (part.flag()&VTX_HLP)) // set only for parts that already have leaf attachment
            {
               part.setRandomLeafBending(random); changed=true;
            }
         }
      }
      if(changed)ObjEdit.setChangedMesh(true, false);
   }
   void LeafRegion::RandomColor(LeafRegion &leaf)
   {
      ObjEdit.mesh_undos.set("color");
      flt  variation=TextFlt(leaf.color_value());
      bool changed=false;
      REPD(i, ObjEdit.mesh.lods())if(ObjEdit.selLod()==i || !ObjEdit.lod_tabs.visibleFull())
      {
         MeshLod &lod=ObjEdit.mesh.lod(i); REPA(lod.parts)if(ObjEdit.partOp(i) || !ObjEdit.mesh_parts.visibleFull())
         {
            MeshPart &part=lod.parts[i]; if(HasMaterial(part, MtrlEdit.game)){part.setRandomLeafColor(variation); changed=true;}
         }
      }
      if(changed)ObjEdit.setChangedMesh(true, false);
   }
   bool LeafRegion::meshHasMtrl(C MaterialPtr &mtrl)
   {
      REPD(i, ObjEdit.mesh.lods()){MeshLod &lod=ObjEdit.mesh.lod(i); REPA(lod.parts)if(HasMaterial(lod.parts[i], mtrl))return true;}
      return false;
   }
   LeafRegion& LeafRegion::create()
   {
      ::EE::Region::create(Rect_LU(0, 0, 0.44f, 0.793f)).skin(&TransparentSkin, false); kb_lit=false;
      ts.reset(); ts.size=0.038f; ts.align.set(1, 0);
      flt h=0.044f, p=0.005f, vh=h*0.97f, y=-0.025f, w=rect().w()-0.02f;
      T+=leaf_attachment.create(Vec2(0.01f, y), "Set Leaf Attachment", &ts); y-=h/2;
      T+=texture.create(Rect(0.01f, y-w, rect().w()-0.01f, y)).desc("Click on the image to set leaf attachment according to selected texture position"); y-=w+h/2; texture.alpha_mode=ALPHA_NONE;
      T+=set_attachment_cam .create(Rect_L(0.01f, y, w     , vh), "Set At Cam Target"      ).func(SetAttachmentCam , T).desc("This function will set Leaf Attachment at current Camera Target"); y-=h;
      T+=remove_attachment  .create(Rect_L(0.01f, y, w     , vh), "Del Leaf Attachment"    ).func(RemoveAttachment , T); y-=h+p;
      T+=     random_bending.create(Rect_L(0.01f, y, w     , vh), "Set Random Bending"     ).func(    RandomBending, T).desc("This function will force each leaf to bend differently"); y-=h;
      T+=same_random_bending.create(Rect_L(0.01f, y, w     , vh), "Set Same Random Bending").func(SameRandomBending, T).desc("This function will set the same random bending value for selected mesh parts"); y-=h;
      T+=     remove_bending.create(Rect_L(0.01f, y, w     , vh), "Del Random Bending"     ).func(    RemoveBending, T); y-=h+p;
      T+=random_color       .create(Rect_L(0.01f, y, w-0.09f, vh), "Set Random Color"       ).func(RandomColor      , T).desc("This function will randomize color of each leaf"); y-=h;
      T+=remove_color       .create(Rect_L(0.01f, y, w-0.09f, vh), "Del Random Color"       ).func(RemoveColor     , T); y-=h;
      T+=color_value        .create(Rect_L(random_color.rect().right()+Vec2(0.01f, 0), 0.08f, vh), "0.3").desc("Random color variation (0..1)");
      return T;
   }
   void LeafRegion::update(C GuiPC &gpc)
{
      ::EE::Region::update(gpc);
      visible(MtrlEdit.visible() && (MtrlEdit.edit.tech==MTECH_LEAF || MtrlEdit.edit.tech==MTECH_BLEND_LIGHT_LEAF || MtrlEdit.edit.tech==MTECH_TEST_BLEND_LIGHT_LEAF) && meshHasMtrl(MtrlEdit.game));
      if(visible())
      {
         texture.image=(MtrlEdit.game ? MtrlEdit.game->base_0 : ImagePtr());
         if(Ms.b(0) && Gui.ms()==&texture)
         {
            Vec2 tex=(Ms.pos()-texture.screenPos())/texture.size(); CHS(tex.y); tex.sat();
            bool changed=false;
            REPD(i, ObjEdit.mesh.lods())if(ObjEdit.selLod()==i || !ObjEdit.lod_tabs.visibleFull())
            {
               MeshLod &lod=ObjEdit.mesh.lod(i); REPA(lod.parts)if(ObjEdit.partOp(i) || !ObjEdit.mesh_parts.visibleFull())
               {
                  MeshPart &part=lod.parts[i]; if(HasMaterial(part, MtrlEdit.game)){ObjEdit.mesh_undos.set("leaf"); part.setLeafAttachment(tex); changed=true;}
               }
            }
            if(changed)ObjEdit.setChangedMesh(true, false);
         }
         if(Ms.bp(2) && contains(Gui.ms()))MtrlEdit.set(null);
      }
   }
/******************************************************************************/
