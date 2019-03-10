/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#pragma pack(push, 1)
struct ICONDIR
{
   UShort reserved, type, images;
};
struct ICONDIRENTRY
{
   Byte   width, height, colors, reserved;
   UShort hot_x, hot_y;
   UInt   size, offset;
};
#pragma pack(pop)
/******************************************************************************/
Bool Image::ImportICO(File &f)
{
   Long    pos=f.pos(); // remember current position in case we're not at the start
   ICONDIR dir; f>>dir;
   if(!Unaligned(dir.reserved) && Unaligned(dir.images)<=256 && (Unaligned(dir.type)==1 || Unaligned(dir.type)==2))
   {
      Bool ico=(Unaligned(dir.type)==1), cur=(Unaligned(dir.type)==2);
      Int  best_x, best_y, best_col;
      UInt best_offset=0, best_size, end=0;
      REP(Unaligned(dir.images))
      {
         ICONDIRENTRY entry; f>>entry;
         if(Unaligned(entry.size)>=f.size() || Unaligned(entry.offset)>f.size() || Unaligned(entry.reserved))goto error;
         Int x=(Unaligned(entry.width ) ? Unaligned(entry.width ) : 256),
             y=(Unaligned(entry.height) ? Unaligned(entry.height) : 256),
             col=(ico ? Unaligned(entry.hot_y) : Unaligned(entry.colors) ? Unaligned(entry.colors) : 256);
         MAX(end, Unaligned(entry.offset)+Unaligned(entry.size));
         if(!best_offset || (x>best_x || y>best_y) || (x==best_x && y==best_x && (col>best_col || (col==best_col && Unaligned(entry.size)>best_size)))) // if we've encountered same size and colors then use the one of bigger raw size
         {
            best_offset=Unaligned(entry.offset);
            best_size  =Unaligned(entry.size  );
            best_x     =x;
            best_y     =y;
            best_col   =col;
         }
      }
      if(best_offset && f.ok())
      {
         if(f.pos(best_offset+pos) && ImportBMPRaw(f, true)){f.pos(end+pos); return true;} // try as BMP and move to the end of the file on success
         if(f.pos(best_offset+pos) && ImportPNG   (f      )){f.pos(end+pos); return true;} // try as PNG and move to the end of the file on success
      }
   }
error:;
   del(); return false;
}
Bool Image::ImportICO(C Str &name)
{
   File f; if(f.readTry(name))return ImportICO(f);
   del(); return false;
}
/******************************************************************************/
struct Mip
{
   VecI2 size;
   File  data;
};
Bool Image::ExportICO(File &f)C
{
   if(!is())return false;

 C Image *src=this;
   Image  temp;

   Bool  legacy=false, full_size_included=false;
   Mip   mips[5]; // up to 5 mip-maps, providing mip-maps with Esenthel's high quality filtering allows the OS to use them instead of its own filtering, one will be used for Windows XP, on Windows we don't need many mip maps
   FREPA(mips)
   {
      Mip &mip=mips[i];
      if(!legacy && (i==Elms(mips)-1 || src->size().max()<96)) // set one mip to be Windows XP compatible (48x48 not compressed)
      {
         legacy=true;
         VecI2 size=src->size();
         if(size.x>48)size=size*48/size.x;
         if(size.y>48)size=size*48/size.y;
         Image xp; if(src->copyTry(xp, Max(1, size.x), Max(1, size.y), 1, IMAGE_B8G8R8A8, IMAGE_SOFT, 1, FILTER_BEST, true, true)) // ICO uses BGRA order
         {
            if(xp.ExportBMPRaw(mip.data.writeMem(), 4, true))
            {
               mip.size=xp.size(); if(xp.size()==T.size())full_size_included=true;
            }else mip.data.del();
         }
      }else
      {
         if(full_size_included)
         {
            VecI2 size=src->size()/2; if(size.max()<16)break;
            if(src->copyTry(temp, Max(1, size.x), Max(1, size.y), 1, ImageTI[src->type()].compressed ? IMAGE_R8G8B8A8 : -1, IMAGE_SOFT, 1, FILTER_BEST, true, true))src=&temp;else break;
         }
         if(src->ExportPNG(mip.data.writeMem(), 1)){mip.size=src->size(); full_size_included=true;}else mip.data.del();
      }
   }

   Int images=0; FREPA(mips)if(mips[i].data.is())images++; if(!images)return false;
   ICONDIR        dir; _Unaligned(dir.reserved, 0); _Unaligned(dir.type, 1); _Unaligned(dir.images, images); f<<dir;
   ICONDIRENTRY entry; _Unaligned(entry.colors, 0); _Unaligned(entry.reserved, 0); _Unaligned(entry.hot_x, 1); _Unaligned(entry.hot_y, 32);
   UInt offset=SIZE(ICONDIR)+SIZE(ICONDIRENTRY)*images;
   FREPA(mips)
   {
      Mip &mip=mips[i]; if(mip.data.is())
      {
        _Unaligned(entry.width , (mip.size.x>=256) ? 0 : mip.size.x);
        _Unaligned(entry.height, (mip.size.y>=256) ? 0 : mip.size.y);
        _Unaligned(entry.size  , mip.data.size());
        _Unaligned(entry.offset, offset);
         f<<entry; 
         offset+=mip.data.size();
      }
   }
   FREPA(mips)
   {
      File &data=mips[i].data; if(data.is())
      {
         data.pos(0); if(!data.copy(f))return false;
      }
   }
   return f.ok();
}
Bool Image::ExportICO(C Str &name)C
{
   File f; if(f.writeTry(name)){if(ExportICO(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
/******************************************************************************/
static void ICNSEncodeRLE24(File &f, C Byte *data, Int size) // Code based on LibICNS - http://icns.sourceforge.net/, assumes that 'data' is in IMAGE_R8G8B8A8 format
{
   // Assumptions of what icns rle data is all about:
   // A) Each channel is encoded indepenent of the next.
   // B) An encoded channel looks like this:
   //    0xRL 0xCV 0xCV 0xRL 0xCV - RL is run-length and CV is color value.
   // C) There are two types of runs
   //    1) Run of same value - high bit of RL is set
   //    2) Run of differing values - high bit of RL is NOT set
   // D) 0xRL also has two ranges
   //    1) for set high bit RL, 3 to 130
   //    2) for clr high bit RL, 1 to 128
   // E) 0xRL byte is therefore set as follows:
   //    1) for same values, RL=RL - 1
   //    2) different values, RL=RL+125
   //    3) both methods will automatically set the high bit appropriately
   // F) 0xCV byte are set accordingly
   //    1) for differing values, run of all differing values
   //    2) for same values, only one byte of that values
   // Estimations put the absolute worst case scenario as the
   // final compressed data being slightly LARGER. So we need to be
   // careful about allocating memory. (Did I miss something?)
   // tests seem to indicate it will never be larger than the original

   // There's always going to be 4 channels in this
   // so we want our counter to increment through
   // channels, not bytes....
   UInt dataInChanSize=size/4;

   // Move forward 4 bytes for big size - who knows why this should be
   if(size>=65536)f.putUInt(0);

   Byte dataRun[256]; Zero(dataRun);

   // Data is stored in red run, green run,blue run
   // So we compress from pixel format RGBA
   // RED:   byte[0], byte[4], byte[8]  ...
   // GREEN: byte[1], byte[5], byte[9]  ...
   // BLUE:  byte[2], byte[6], byte[10] ...
   // ALPHA: byte[3], byte[7], byte[11] do nothing with these bytes

   for(Byte colorOffset=0; colorOffset<3; colorOffset++)
   {
      Int runCount=0;

      // Set the first byte of the run...
      dataRun[0]=*(data+colorOffset);

      // Start with a runlength of 1 for the first byte
      Byte runLength=1; // Runs will never go over 130, one byte is ok

      // Assume that the run will be different for now... We can change this later
      Bool runType=false; // 0 for low bit (different), 1 for high bit (same)	

      // Start one byte ahead
      for(UInt dataInCount=1; dataInCount<dataInChanSize; dataInCount++)
      {
         Byte dataByte=*(data+colorOffset+(dataInCount*4));

         if(runLength<2)
         {
            // Simply append to the current run
            dataRun[runLength++]=dataByte;
         }else
         if(runLength==2)
         {
            // Decide here if the run should be same values or different values
            // If the last three values were the same, we can change to a same-type run
            if(dataByte==dataRun[runLength-1] && dataByte==dataRun[runLength-2])runType=1;else runType=0;
            dataRun[runLength++]=dataByte;
         }else // Greater than or equal to 2
         {
            if(runType==0 && runLength<128) // Different type run
            {
               // If the new value matches both of the last two values, we have a new
               // same-type run starting with the previous two bytes
               if(dataByte==dataRun[runLength-1] && dataByte==dataRun[runLength-2])
               {
                  // Set the RL byte
                  f.putByte(runLength-3);

                  // Copy 0 to runLength-2 bytes to the RLE data here
                  f.put(dataRun, runLength-2);
                  runCount++;

                  // Set up the new same-type run
                  dataRun[0]=dataRun[runLength-2];
                  dataRun[1]=dataRun[runLength-1];
                  dataRun[2]=dataByte;
                  runLength=3;
                  runType=1;
               }else // They don't match, so we can proceed
               {
                  dataRun[runLength++]=dataByte;
               }
            }else
            if(runType==1 && runLength<130) // Same type run
            {
               // If the new value matches both of the last two values, we
               // can safely continue
               if(dataByte==dataRun[runLength-1] && dataByte==dataRun[runLength-2])
               {
                  dataRun[runLength++]=dataByte;
               }else // They don't match, so we need to start a new run
               {
                  // Set the RL byte
                  f.putByte(runLength+125);

                  // Only copy the first byte, since all the remaining values are identical
                  f.putByte(dataRun[0]);
                  runCount++;

                  // Copy 0 to runLength bytes to the RLE data here
                  dataRun[0]=dataByte;
                  runLength=1;
                  runType=0;
               }
            }else // Exceeded run limit, need to start a new one
            {
               if(runType==0)
               {
                  // Set the RL byte low
                  f.putByte(runLength-1);

                  // Copy 0 to runLength bytes to the RLE data here
                  f.put(dataRun, runLength);
               }else
               if(runType==1)
               {
                  // Set the RL byte high
                  f.putByte(runLength+125);

                  // Only copy the first byte, since all the remaining values are identical
                  f.putByte(dataRun[0]);
               }
               runCount++;

               // Copy 0 to runLength bytes to the RLE data here
               dataRun[0]=dataByte;
               runLength=1;
               runType=0;
            }
         }
      }

      // Copy the end of the last run
      if(runLength>0)
      {
         if(runType==0)
         {
            // Set the RL byte low
            f.putByte(runLength-1);

            // Copy 0 to runLength bytes to the RLE data here
            f.put(dataRun, runLength);
         }else
         if(runType==1)
         {
            // Set the RL byte high
            f.putByte(runLength+125);

            // Only copy the first byte, since all the remaining values are identical
            f.putByte(dataRun[0]);
         }
         runCount++;
      }
   }
}
struct MipAlpha : Mip
{
   Bool png;
   File alpha;
};
Bool Image::ExportICNS(File &f)C // ICNS stores data using RLE, PNG or JPEG 2000 (following implementation uses RLE and PNG), format is big-endian
{
   if(!is())return false;

 C Image *src=this;
   Image  temp;

   // Mac OS X (as of 10.10) has following issues that affect opening of apps with those icons (this doesn't apply to opening just the icon themselves)
   // - setting up 1024     size        will make it ignored      (maybe it's because it's "@2x retina"
   // - setting up 16,32,64 size as PNG will make it look corrupt (maybe it's related to 'icp*' instead of 'ic0*' however those were tried too and failed)
   Int size=Mid(NearestPow2(src->size().avgI()), 16, 512);
   if(size==64)size=128; // because 64 size can't be used as PNG, it has to be done as RLE, and there it is processed as 48, which will be very low res, so use 128 as the max in that case
   if(src->w()!=size || src->h()!=size)if(src->copyTry(temp, size, size, 1, ImageTI[src->type()].compressed ? IMAGE_R8G8B8A8 : -1, IMAGE_SOFT, 1, FILTER_BEST, true, true))src=&temp;else return false;
   MipAlpha mips[3]; // up to 3 mip-maps, providing mip-maps with Esenthel's high quality filtering allows the OS to use them instead of its own filtering
   FREPA(mips)
   {
      MipAlpha &mip=mips[i];
      if(i)
      {
         Int s=src->w()/2; if(s<16)break;
         if(src->copyTry(temp, s, s, 1, ImageTI[src->type()].compressed ? IMAGE_R8G8B8A8 : -1, IMAGE_SOFT, 1, FILTER_BEST, true, true))src=&temp;else break;
      }
      if(src->w()<=64) // RLE
      {
         Int    size=src->w(); if(size>128)size=128;else if(size==64)size=48; // RLE doesn't support sizes >128 and 64
       C Image *s=src;
         Image  rle; 
         if(s->hwType()!=IMAGE_R8G8B8A8 || s->w()!=size || s->size()!=s->hwSize()) // 'ICNSEncodeRLE24' requires IMAGE_R8G8B8A8 without any hw padding
            if(s->copyTry(rle, size, size, 1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1, FILTER_BEST, true, true))s=&rle;else continue;
         if(s->lockRead())
         {
            mip.png=false;
            mip.size=s->size();
            ICNSEncodeRLE24(mip.data.writeMem(), s->data(), s->pitch2());
            mip.alpha.writeMem();
            FREPD(y, s->h())
            FREPD(x, s->w())mip.alpha.putByte(s->color(x, y).a);
            s->unlock();
         }
      }else // PNG
      {
         if(src->ExportPNG(mip.data.writeMem(), 1))
         {
            mip.png=true;
            mip.size=src->size();
         }else mip.data.del();
      }
   }

   // header
   f.putUInt(CC4('i', 'c', 'n', 's'));
   UInt u=4+4; FREPA(mips)
   {
      MipAlpha &mip=mips[i];
      if(mip.data .is())u+=4+4 + mip.data .size();
      if(mip.alpha.is())u+=4+4 + mip.alpha.size();
   }
   if(u<=4+4)return false; // no mips
   SwapEndian(u); f.putUInt(u); // (ICNS header) + (Image header + Image data)

   // mips
   FREPA(mips)
   {
      MipAlpha &mip=mips[i];
      if(mip.data.is())
      {
         // mip header
         if(mip.png)switch(mip.size.x)
         {
          //case   16: f.putUInt(CC4('i', 'c', 'p', '4')); break; corrupt
          //case   32: f.putUInt(CC4('i', 'c', 'p', '5')); break; corrupt
          //case   64: f.putUInt(CC4('i', 'c', 'p', '6')); break; corrupt
            case  128: f.putUInt(CC4('i', 'c', '0', '7')); break;
            case  256: f.putUInt(CC4('i', 'c', '0', '8')); break;
            case  512: f.putUInt(CC4('i', 'c', '0', '9')); break;
          //case 1024: f.putUInt(CC4('i', 'c', '1', '0')); break; ignored
         }else
         switch(mip.size.x)
         {
            case  16: f.putUInt(CC4('i', 's', '3', '2')); break;
            case  32: f.putUInt(CC4('i', 'l', '3', '2')); break;
            case  48: f.putUInt(CC4('i', 'h', '3', '2')); break;
          //case 128: f.putUInt(CC4('i', 't', '3', '2')); break; currently always saved as PNG
         }
         u=4+4 + mip.data.size(); SwapEndian(u); f.putUInt(u);

         // mip data
         mip.data.pos(0); if(!mip.data.copy(f))return false;
      }
      if(mip.alpha.is())
      {
         // mip header
         switch(mip.size.x)
         {
            case  16: f.putUInt(CC4('s', '8', 'm', 'k')); break;
            case  32: f.putUInt(CC4('l', '8', 'm', 'k')); break;
            case  48: f.putUInt(CC4('h', '8', 'm', 'k')); break;
          //case 128: f.putUInt(CC4('t', '8', 'm', 'k')); break; currently always saved as PNG
         }
         u=4+4 + mip.alpha.size(); SwapEndian(u); f.putUInt(u);

         // mip data
         mip.alpha.pos(0); if(!mip.alpha.copy(f))return false;
      }
   }

   return f.ok();
}
Bool Image::ExportICNS(C Str &name)C
{
   File f; if(f.writeTry(name)){if(ExportICNS(f) && f.flush())return true; f.del(); FDelFile(name);}
   return false;
}
/******************************************************************************/
}
/******************************************************************************/
