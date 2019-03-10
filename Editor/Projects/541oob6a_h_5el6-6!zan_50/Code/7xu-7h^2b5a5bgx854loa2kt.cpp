/******************************************************************************/
class CompressedSound : SoundCallback
{
   Str         file=EncodeFileName(UID(4114948924, 1122167854, 3363163781, 3122373663));
   SoundStream stream;
   OpusEncoder encoder;
   OpusDecoder decoder;

   Memc<byte>  decoded_data;
   int         decoded_pos=0;

   virtual bool create(byte &bits, byte &channels, int &frequency, long &raw_length, int &bit_rate)override // this will be called upon first usage of the callback, Warning: this may get called on a secondary thread
   {
      if(stream.create(file))
      if(encoder.create(stream.channels(), false))if(encoder.bits()==stream.bits())
      if(decoder.create(stream.channels()))
      {
         bits=decoder.bits();
         channels=decoder.channels();
         frequency=stream.frequency(); // use stream frequency because decoder will always be at OPUS_FREQUENCY
         
             BitRateChanged(); // set bit   rate   settings from gui
         FrameLengthChanged(); // set frame length settings from gui
         return true;
      }
      return false;
   }

   virtual int set(ptr data, int size)override // this will be called when 'data' needs to be filled with uncompressed sound data, Warning: this may get called on a secondary thread
   {
      int written=0;
      if(Mode()==0) // original
      {
         written=stream.set(data, size);
         if(written<=0) // if nothing was written, possibly we've reached the end of the stream
         {
            stream.pos(0); // reset position
            written=stream.set(data, size); // try again
         }
      }else // compressed
      {
      start:
         if(!decoded_data.elms()) // if there's no decoded data yet
         {
            // load data from the stream
            byte temp[4096];
            int loaded=stream.set(temp, SIZE(temp));
            if(loaded<=0) // if nothing was written, possibly we've reached the end of the stream
            {
               stream.pos(0); // reset position
               loaded=stream.set(temp, SIZE(temp)); // try again
            }
            if(loaded<=0)return loaded; // error occurred
            
            // compress that data
            Memt<byte> compressed;
            Memt<int > packet_sizes;
            if(!encoder.encode(temp, loaded, compressed, packet_sizes))return -1;

            // decompress packets
            int compressed_pos=0;
            Memt<byte> decompressed;
            FREPA(packet_sizes)
            {
               if(!decoder.decode(&compressed[compressed_pos], packet_sizes[i], decompressed))return -1;
               compressed_pos+=packet_sizes[i];
               Copy(&decoded_data[decoded_data.addNum(decompressed.elms())], decompressed.data(), decompressed.elms()); // copy decompressed packet into
            }

            goto start; // go to the start in case no packets were yet created
         }
         int decoded_left=decoded_data.elms()-decoded_pos; // get number of decoded data left
         written=Min(size, decoded_left); // minimize it with needed data
         Copy(data, &decoded_data[decoded_pos], written); // copy that data
         decoded_pos+=written; // increment decoded data position
         if(decoded_pos>=decoded_data.elms()){decoded_pos=0; decoded_data.clear();} // if we've copied entire 'decoded_data' then reset its state
      }
      return written;
   }
}
CompressedSound cs;
/******************************************************************************/
Sound     sound;
Tabs      Mode;
Text     TBitRate, TFrameLength, TDragDrop;
ComboBox  BitRate,  FrameLength;
cchar8   *BitRates[]=
{
     "8000",
    "16000",
    "32000",
    "48000",
    "64000",
    "96000",
   "128000",
};
cchar8 *FrameLengths[]=
{
   "2.5",
   "5",
   "10",
   "20",
   "40",
   "60",
};
void     BitRateChanged(ptr user=null) {cs.encoder.bitRate    (TextInt(BitRate    .text));}
void FrameLengthChanged(ptr user=null) {cs.encoder.frameLength(TextFlt(FrameLength.text));}
void FileDropped(Memc<Str> &names, GuiObj *focus_obj, C Vec2 &screen_pos) {if(names.elms()){cs.file=names[0]; sound.play(cs);}}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   D.scale(1);
   App.drop=FileDropped;
   App.flag|=APP_WORK_IN_BACKGROUND|APP_MINIMIZABLE;
}
/******************************************************************************/
bool Init()
{
   cchar8 *modes[]=
   {
      "Original"  ,
      "Compressed",
   };
   Gui+=Mode.create(Rect_C(0, 0.4, 1.0, 0.09), 0, modes, Elms(modes)).set(1).valid(true);

   Mode.tab(1)+=TBitRate.create(Vec2  (-0.3, 0.05), "Bit Rate", Gui.skin->text_style);
   Mode.tab(1)+= BitRate.create(Rect_U(-0.3, 0.0, 0.4, 0.09), BitRates, Elms(BitRates)).func(BitRateChanged, null, true).set(4);

   Mode.tab(1)+=TFrameLength.create(Vec2  (0.3, 0.09), "Frame Length\n(latency in milliseconds)", Gui.skin->text_style);
   Mode.tab(1)+= FrameLength.create(Rect_U(0.3, 0.0, 0.4, 0.09), FrameLengths, Elms(FrameLengths)).func(FrameLengthChanged, null, true).set(3);

   Gui+=TDragDrop.create(Vec2(0, 0.8), "Drag and Drop a sound file on this window to play it", Gui.skin->text_style);

   sound.play(cs);
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
   Gui.update();
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   D.text(Rect(Vec2(0, -0.5)), S+cs.file+"\nLength: "+Round(cs.stream.length())+"s, BitRate: "+cs.stream.bitRate()+", Frequency: "+cs.stream.frequency()+", Channels: "+cs.stream.channels()+", Codec: "+cs.stream.codecName());
   Gui.draw();
}
/******************************************************************************/
