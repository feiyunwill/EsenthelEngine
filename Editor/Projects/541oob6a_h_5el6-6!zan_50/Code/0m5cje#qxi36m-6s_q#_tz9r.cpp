/******************************************************************************/
Video video      , // video for RGB colors
      video_alpha; // video for Alpha only (from greyscale source)
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   if(!video      .create(UID(352823356, 1075274726, 2906837907, 348097182)                    ))Exit("Can't create video"); // create video from file
   if(!video_alpha.create(UID(352823356, 1075274726, 2906837907, 348097182), false, Video.ALPHA))Exit("Can't create video"); // create video from file as ALPHA mode
   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   video      .update(Time.stateTime()); // update video to current state time
   video_alpha.update(Time.stateTime()); // update video to current state time

   return true;
}
/******************************************************************************/
void Draw()
{
   D    .clear      (TURQ);
   video.drawAlphaFs(video_alpha); // draw video with alpha from 'video_alpha'
   D    .text       (0, 0.9, S+video.time()); // draw time position of video
}
/******************************************************************************/
