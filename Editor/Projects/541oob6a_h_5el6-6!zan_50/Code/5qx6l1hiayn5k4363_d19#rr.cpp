/******************************************************************************/
class Decal2 : Decal // create a helper class which bases on Decal and uses time fading
{
   flt time=10; // time left until overlay fades out

   bool update()
   {
             time-=Time.d(); // decrease time left
      return time>0        ; // return if there's still time left before it should be deleted
   }
   void draw(C Matrix &matrix)
   {
      super.drawAnimated(matrix, Sat(time)); // draw the overlay with transparency of 'Saturate(time)' value
   }

   // io
   bool save(File &f)
   {
      if(super.save(f))
      {
         f<<time;
         return f.ok();
      }
      return false;
   }
   bool load(File &f)
   {
      if(super.load(f))
      {
         f>>time;
         if(f.ok())return true;
      }
      return false;
   }
}
/******************************************************************************/
class Item : Game.Item // extend items
{
   Memc<Decal2> decals; // add decal container, which is used for rendering semi transparent images on solid surfaces

   virtual bool update() // extend updating to include 'decals' update
   {
      if(super.update())
      {
         REPA(decals)if(!decals[i].update()) // update all overlays
            decals.remove(i, true); // and remove them if they faded out

         return true;
      }
      return false;
   }
   virtual uint drawPrepare() // extend drawing  to include request for the RM_OVERLAY mode
   {
      uint modes=super.drawPrepare();
      if(decals.elms())modes|=IndexToFlag(RM_OVERLAY); // include request for additional render mode
      return modes;
   }
   virtual void drawOverlay() // extend drawing  to include rendering of 'decals'
   {
      REPAO(decals).draw(matrixScaled()); // draw mesh_overlays with the same matrix used for default item drawing
   }

   // io
   virtual bool save(File &f) // extend saving  to include members saving
   {
      if(super.save(f))
      {
         if(decals.save(f))
            return f.ok();
      }
      return false;
   }
   virtual bool load(File &f) // extend loading to include members loading
   {
      if(super.load(f))
      {
         if(decals.load(f))
            if(f.ok())return true;
      }
      return false;
   }

   // operations
   void addBulletHole(C Vec &pos, C Vec &surface_normal, C Vec &shot_dir) // helper method for adding a bullet hole (Decal) onto the item mesh
   {
      Matrix bullet_matrix; bullet_matrix.setPosDir(pos, surface_normal).scaleOrn(0.05);

      // add a decal to the item
      Decal &decal=decals.New();
      decal.material (bullet_mtrl_id);
      decal.setMatrix(matrixScaled(), bullet_matrix);

      // add impulse to the actor
      actor.addImpulse(shot_dir*1.5, pos);
   }
}
/******************************************************************************/
