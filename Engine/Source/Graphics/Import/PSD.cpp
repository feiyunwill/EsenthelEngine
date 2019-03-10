/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#if SUPPORT_PSD
struct PSD
{
   struct HEADER
   {
      Byte Signature[4], // always equal 8BPS, do not read file if not
           Version  [2], // always equal 1, do not read file if not
           Reserved [6], // must be zero
           Channels [2], // numer of channels including any alpha channels, supported range 1 to 24
           Rows     [4], // height in PIXELS, supported range 1 to 30000
           Columns  [4], // width in PIXELS, supported range 1 to 30000
           Depth    [2], // number of bpp
           Mode     [2]; // colour mode of the file, Bitmap=0, Grayscale=1, Indexed=2, RGB=3, CMYK=4, Multichannel=7, Duotone=8, Lab=9
   };

   struct HEADER_INFO
   {
      //Table 2-12: HeaderInfo Color spaces
      //	Color-ID	Name	Description
      //-------------------------------------------
      //		0		Bitmap			// Probably means black & white
      //		1		Grayscale		The first value in the color data is the gray value, from 0...10000.
      //		2		Indexed
      //		3		RGB				The first three values in the color data are red, green, and blue.
      //								They are full unsigned 16-bit values as in Apple's RGBColor data
      //								structure. Pure red=65535,0,0.
      //		4		CMYK			The four values in the color data are cyan, magenta, yellow, and
      //								black. They are full unsigned 16-bit values. 0=100% ink. Pure
      //								cyan=0,65535,65535,65535.
      //		7		Multichannel	// Have no idea
      //		8		Duotone
      //		9		Lab				The first three values in the color data are lightness, a chrominance,
      //								and b chrominance.
      //								Lightness is a 16-bit value from 0...100. The chromanance components
      //								are each 16-bit values from -128...127. Gray values
      //								are represented by chrominance components of 0. Pure
      //								white=100,0,0.
      I16 nChannels;
      Int nHeight;
      Int nWidth;
      I16 nBitsPerPixel;
      I16 nColourMode;

      HEADER_INFO()
      {
         nChannels=-1;
         nHeight=-1;
         nWidth=-1;
         nBitsPerPixel=-1;
         nColourMode=-1;
      }
   };

   struct COLOUR_MODE_DATA
   {
      Int   nLength;
      Byte *ColourData;

     ~COLOUR_MODE_DATA()
      {
         DeleteN(ColourData);
      }
      COLOUR_MODE_DATA()
      {
         nLength=-1;
         ColourData=null;
      }
   };

   struct IMAGE_RESOURCE
   {
      // Table 2-1: Image resource block
      //	Type		Name	Description
      //-------------------------------------------
      //	OSType		Type	Photoshop always uses its signature, 8BIM
      //	int16		ID		Unique identifier
      //	PString		Name	A pascal string, padded to make size even (a null name consists of two bytes of 0)
      //						Pascal style string where the first byte gives the length of the
      //						string and the content bytes follow.
      //	int32		Size	Actual size of resource data. This does not include the
      //						Type, ID, Name, or Size fields.
      //	Variable	Data	Resource data, padded to make size even

      Int   nLength;
      char  OSType[4];
      I16   nID;
      Byte *Name;
      Int	nSize;

      IMAGE_RESOURCE()
      {
         Name=null;
         Reset();
      }
     ~IMAGE_RESOURCE()
      {
         DeleteN(Name);
      }

      void Reset()
      {
         nLength=-1;
         REPAO(OSType)=0;
         nID=-1;
         DeleteN(Name);
         nSize=-1;
      }
   };

   struct RESOLUTION_INFO
   {
      // Table A-6: ResolutionInfo structure
      //	Type		Name	Description
      //-------------------------------------------
      //	Fixed		hRes		Horizontal resolution in pixels per inch.
      //	Int			hResUnit	1=display horizontal resolution in pixels per inch;
      //							2=display horizontal resolution in pixels per cm.
      //	I16		widthUnit	Display width as 1=inches; 2=cm; 3=points; 4=picas; 5=columns.
      //	Fixed		vRes		Vertical resolution in pixels per inch.
      //	Int			vResUnit	1=display vertical resolution in pixels per inch;
      //							2=display vertical resolution in pixels per cm.
      //	I16		heightUnit	Display height as 1=inches; 2=cm; 3=points; 4=picas; 5=columns.
      I16 hRes;
      Int hResUnit;
      I16 widthUnit;

      I16 vRes;
      Int vResUnit;
      I16 heightUnit;

      RESOLUTION_INFO()
      {
         hRes=-1;
         hResUnit=-1;
         widthUnit=-1;
         vRes=-1;
         vResUnit=-1;
         heightUnit=-1;
      }
   };

   struct RESOLUTION_INFO_v2	// Obsolete - Photoshop 2.0
   {
      I16 nChannels;
      I16 nRows;
      I16 nColumns;
      I16 nDepth;
      I16 nMode;
      RESOLUTION_INFO_v2()
      {
         nChannels=-1;
         nRows=-1;
         nColumns=-1;
         nDepth=-1;
         nMode=-1;
      }
   };

   struct DISPLAY_INFO
   {
      // This structure contains display information about each channel.
      //Table A-7: DisplayInfo Color spaces
      //	Color-ID	Name	Description
      //-------------------------------------------
      //		0		RGB			The first three values in the color data are red, green, and blue.
      //							They are full unsigned 16-bit values as in Apple's RGBColor data
      //							structure. Pure red=65535,0,0.
      //		1		HSB			The first three values in the color data are hue, saturation, and
      //							brightness. They are full unsigned 16-bit values as in Apple's
      //							HSVColor data structure. Pure red=0,65535, 65535.
      //		2		CMYK		The four values in the color data are cyan, magenta, yellow, and
      //							black. They are full unsigned 16-bit values. 0=100% ink. Pure
      //							cyan=0,65535,65535,65535.
      //		7		Lab			The first three values in the color data are lightness, a chrominance,
      //							and b chrominance.
      //							Lightness is a 16-bit value from 0...10000. The chromanance components
      //							are each 16-bit values from -12800...12700. Gray values
      //							are represented by chrominance components of 0. Pure
      //							white=10000,0,0.
      //		8		grayscale	The first value in the color data is the gray value, from 0...10000.
      I16  ColourSpace;
      I16  Colour[4];
      I16  Opacity;			// 0..100
      Bool kind;				// selected=0, protected=1
      Byte padding;	// should be zero

      DISPLAY_INFO()
      {
         ColourSpace=-1;
         REPAO(Colour)=0;
         Opacity=-1;
         kind=false;
         padding='0';
      }
   };

   struct THUMBNAIL
   {
      // Adobe Photoshop 5.0 and later stores thumbnail information for preview
      // display in an image resource block. These resource blocks consist of an
      // 28 byte header, followed by a JFIF thumbnail in RGB (red, green, blue)
      // for both Macintosh and Windows. Adobe Photoshop 4.0 stored the
      // thumbnail information in the same format except the data section is
      // (blue, green, red). The Adobe Photoshop 4.0 format is at resource ID
      // and the Adobe Photoshop 5.0 format is at resource ID 1036.
      // Table 2-5: Thumnail resource header
      //	Type		Name		Description
      //-------------------------------------------
      //	4 bytes		format			= 1 (kJpegRGB). Also supports kRawRGB (0).
      //	4 bytes		width			Width of thumbnail in pixels.
      //	4 bytes		height			Height of thumbnail in pixels.
      //	4 bytes		widthbytes		Padded row bytes as (width * bitspixel + 31) / 32 * 4.
      //	4 bytes		size			Total size as widthbytes * height * planes
      //	4 bytes		compressedsize	Size after compression. Used for consistentcy check.
      //	2 bytes		bitspixel		= 24. Bits per pixel.
      //	2 bytes		planes			= 1. Number of planes.
      //	Variable	Data			JFIF data in RGB format.
      //								Note: For resource ID 1033 the data is in BGR format.
      Int	nFormat;
      Int	nWidth;
      Int	nHeight;
      Int	nWidthBytes;
      Int	nSize;
      Int	nCompressedSize;
      I16	nBitPerPixel;
      I16	nPlanes;
      Byte *Data;

      THUMBNAIL()
      {
         nFormat=-1;
         nWidth=-1;
         nHeight=-1;
         nWidthBytes=-1;
         nSize=-1;
         nCompressedSize=-1;
         nBitPerPixel=-1;
         nPlanes=-1;
         Data=null;
      }
   };

   Bool ReadHeader(File &f, HEADER_INFO& header_info);
   Bool ReadColourModeData(File &f, COLOUR_MODE_DATA& colour_mode_data);
   Bool ReadImageResource(File &f, IMAGE_RESOURCE& image_resource);
   Bool ReadLayerAndMaskInfoSection(File &f);
   Bool ReadImageData(File &f, Image &dest);
   Bool ProccessBuffer(Byte *pData, Image &dest);

   HEADER_INFO header_info;

   COLOUR_MODE_DATA colour_mode_data;
   I16 mnColourCount;
   I16 mnTransparentIndex;

   IMAGE_RESOURCE image_resource;

   Int mnGlobalAngle;

   RESOLUTION_INFO resolution_info;
   Bool mbResolutionInfoFilled;

   RESOLUTION_INFO_v2 resolution_info_v2;
   Bool mbResolutionInfoFilled_v2;

   DISPLAY_INFO display_info;
   Bool mbDisplayInfoFilled;

   THUMBNAIL thumbnail;
   Bool mbThumbNailFilled;

   Bool mbCopyright;

   Bool merged_alpha;

   PSD()
   {
      mbThumbNailFilled=false;
      mbDisplayInfoFilled=false;
      mbResolutionInfoFilled=false;
      mbResolutionInfoFilled_v2=false;
      mnGlobalAngle=30;
      mbCopyright=false;
      mnColourCount=-1;
      mnTransparentIndex=-1;
      merged_alpha=false;
   }
};
/******************************************************************************/
static UInt Calculate(Byte *b, Int digs)
{
   UInt   v=0; FREP(digs)v=(v<<8)|b[i];
   return v;
}
static UInt GetUInt(File &f) {Byte b[4]; f>>b; return      Calculate(b, Elms(b));}
static UInt GetU16 (File &f) {Byte b[2]; f>>b; return      Calculate(b, Elms(b));}
static  Int GetI16 (File &f) {Byte b[2]; f>>b; return (I16)Calculate(b, Elms(b));}

static void XYZToRGB(Dbl x, Dbl y, Dbl z, Int &r, Int &g, Int &b)
{
   // Standards used Observer=2, Illuminant=D65
   // ref_X=95.047, ref_Y=100.000, ref_Z=108.883
   Dbl ref_X= 95.047,
       ref_Y=100.000,
       ref_Z=108.883;

   Dbl var_X=x/100.0,
       var_Y=y/100.0,
       var_Z=z/100.0;

   Dbl var_R=var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986,
       var_G=var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415,
       var_B=var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570;

   if(var_R>0.0031308)var_R=1.055*Pow(var_R, 1/2.4)-0.055;
   else               var_R=12.92*var_R;

   if(var_G>0.0031308)var_G=1.055*Pow(var_G, 1/2.4)-0.055;
   else               var_G=12.92*var_G;

   if(var_B>0.0031308)var_B=1.055*Pow(var_B, 1/2.4)-0.055;
   else               var_B=12.92*var_B;

   r=FltToByte(var_R);
   g=FltToByte(var_G);
   b=FltToByte(var_B);
}
static void LABToRGB(Dbl L, Dbl A, Dbl B, Int &r, Int &g, Int &b)
{
   // For the conversion we first convert values to XYZ and then to RGB
   // Standards used Observer=2, Illuminant=D65
   // ref_X=95.047, ref_Y=100.000, ref_Z=108.883
   Dbl ref_X= 95.047,
       ref_Y=100.000,
       ref_Z=108.883;

   Dbl var_Y=(L + 16.0)/116.0,
       var_X= A        /500.0 + var_Y,
       var_Z=var_Y - B /200.0;

   if(Pow(var_Y, 3)>0.008856)var_Y=Pow(var_Y, 3);
   else                      var_Y=(var_Y-16/116)/7.787;

   if(Pow(var_X, 3)>0.008856)var_X=Pow(var_X, 3);
   else                      var_X=(var_X-16/116)/7.787;

   if(Pow(var_Z, 3)>0.008856)var_Z=Pow(var_Z, 3);
   else                      var_Z=(var_Z-16/116)/7.787;

   Dbl x=ref_X*var_X,
       y=ref_Y*var_Y,
       z=ref_Z*var_Z;

   XYZToRGB(x, y, z, r, g, b);
}
static void CMYKToRGB(Dbl c, Dbl m, Dbl y, Dbl k, Int &r, Int &g, Int &b)
{
   //r=RoundPos(Sat(1-(c*(1-k)+k))*255.0f);
   //g=RoundPos(Sat(1-(m*(1-k)+k))*255.0f);
   //b=RoundPos(Sat(1-(y*(1-k)+k))*255.0f);

   Dbl colors=1-k;
   r=RoundPos(Sat(colors*(1-c))*255);
   g=RoundPos(Sat(colors*(1-m))*255);
   b=RoundPos(Sat(colors*(1-y))*255);

   /*r=RoundPos(Sat(1-Min(1.0, c+k))*255);
   g=RoundPos(Sat(1-Min(1.0, m+k))*255);
   b=RoundPos(Sat(1-Min(1.0, y+k))*255);*/

   /*r=RoundPos(255*Sat(1-Min(1.0, c*(1-k)+k)));
   g=RoundPos(255*Sat(1-Min(1.0, m*(1-k)+k)));
   b=RoundPos(255*Sat(1-Min(1.0, y*(1-k)+k)));*/
}
/******************************************************************************/
Bool PSD::ReadHeader(File &f, HEADER_INFO& header_info)
{
   HEADER header; f>>header;
   if(header.Signature[0]=='8' && header.Signature[1]=='B' && header.Signature[2]=='P' && header.Signature[3]=='S')
   {
      Int ver=Calculate(header.Version, SIZE(header.Version));
      if( ver==1)
      {
         REPA(header.Reserved)if(header.Reserved[i])return false;
         header_info.nChannels    =Calculate(header.Channels, SIZE(header.Channels));
         header_info.nHeight      =Calculate(header.Rows    , SIZE(header.Rows    ));
         header_info.nWidth       =Calculate(header.Columns , SIZE(header.Columns ));
         header_info.nBitsPerPixel=Calculate(header.Depth   , SIZE(header.Depth   ));
         header_info.nColourMode  =Calculate(header.Mode    , SIZE(header.Mode    ));
         return f.ok();
      }
   }
   return false;
}
/******************************************************************************/
Bool PSD::ReadColourModeData(File &f, COLOUR_MODE_DATA& colour_mode_data)
{
   // Only indexed colour and duotone have colour mode data,
   // for all other modes this section is 4 bytes length, the length field is set to zero

   // For indexed color images, the length will be equal to 768, and the color
   // will contain the color table for the image, in non-interleaved order.

   // For duotone images, the color data will contain the duotone specification,
   // the format of which is not documented. Other applications that read
   // Photoshop files can treat a duotone image as a grayscale image, and just
   // preserve the contents of the duotone information when reading and writing
   // the file.

   // free memory
   DeleteN(colour_mode_data.ColourData);

      colour_mode_data.nLength=GetUInt(f);
   if(colour_mode_data.nLength>0)
   {
      colour_mode_data.ColourData=new Byte[colour_mode_data.nLength];
      f.get(colour_mode_data.ColourData, colour_mode_data.nLength);
   }
   return f.ok();
}
/******************************************************************************/
Bool PSD::ReadLayerAndMaskInfoSection(File &f)
{
   UInt size=GetUInt(f);
   Long pos=f.pos()+size;
   if(size)
      if(UInt layer_len=GetUInt(f))
   {
      Int layers=GetI16(f);
      if( layers<0)
      {
         merged_alpha=true;
         CHS(layers);
      }
   }
   f.pos(pos);
   return f.ok();
}
/******************************************************************************/
Bool PSD::ReadImageResource(File &f, IMAGE_RESOURCE& image_resource)
{
   image_resource.nLength=GetUInt(f);
#if 1
   f.skip(image_resource.nLength);
   return f.ok();
#else
   Int nBytesRead=0,
       nTotalBytes=image_resource.nLength;

   for(; !f.end() && nBytesRead<nTotalBytes; )
   {
      image_resource.Reset();

      nBytesRead+=f.getReturnSize(image_resource.OSType, SIZE(image_resource.OSType));

      assert(nBytesRead%2==0);
      if(image_resource.OSType[0]=='8' && image_resource.OSType[1]=='B' && image_resource.OSType[2]=='I' && image_resource.OSType[3]=='M')
      {
         Byte ID[2];
         nItemsRead=fread(&ID, SIZE(ID), 1, pFile);
         nBytesRead += nItemsRead * SIZE(ID);

         image_resource.nID=Calculate(ID, SIZE(ID));

         Byte SizeOfName;
         nItemsRead=fread(&SizeOfName, SIZE(SizeOfName), 1, pFile);
         nBytesRead += nItemsRead * SIZE(SizeOfName);

         Int nSizeOfName=Calculate(&SizeOfName, SIZE(SizeOfName));
         if(0<nSizeOfName)
         {
            image_resource.Name=new Byte[nSizeOfName];
            nItemsRead=fread(image_resource.Name, nSizeOfName, 1, pFile);
            nBytesRead += nItemsRead * nSizeOfName;
         }

         if(0 == (nSizeOfName % 2))
         {
            nItemsRead=fread(&SizeOfName, SIZE(SizeOfName), 1, pFile);
            nBytesRead += nItemsRead * SIZE(SizeOfName);
         }

         Byte Size[4];
         nItemsRead=fread(&Size, SIZE(Size), 1, pFile);
         nBytesRead += nItemsRead * SIZE(Size);

         image_resource.nSize=Calculate(Size, SIZE(image_resource.nSize));

         if(0 != (image_resource.nSize % 2))image_resource.nSize++;	// resource data must be even
         
         if(0<image_resource.nSize)
         {
            Byte IntValue[4];
            Byte ShortValue[2];

            switch(image_resource.nID)
            {
               case 1000:
               {
                  // Obsolete - Photoshop 2.0
                  mbResolutionInfoFilled_v2=true;

                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  resolution_info_v2.nChannels=Calculate(ShortValue, SIZE(resolution_info_v2.nChannels));
                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  resolution_info_v2.nRows=Calculate(ShortValue, SIZE(resolution_info_v2.nRows));
                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  resolution_info_v2.nColumns=Calculate(ShortValue, SIZE(resolution_info_v2.nColumns));
                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  resolution_info_v2.nDepth=Calculate(ShortValue, SIZE(resolution_info_v2.nDepth));
                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  resolution_info_v2.nMode=Calculate(ShortValue, SIZE(resolution_info_v2.nMode));
               }break;

               case 1005:
               {
                  mbResolutionInfoFilled=true;

                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  resolution_info.hRes=Calculate(ShortValue, SIZE(resolution_info.hRes));
                  nItemsRead=fread(&IntValue, SIZE(IntValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(IntValue);
                  resolution_info.hResUnit=Calculate(IntValue, SIZE(resolution_info.hResUnit));
                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  resolution_info.widthUnit=Calculate(ShortValue, SIZE(resolution_info.widthUnit));

                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  resolution_info.vRes=Calculate(ShortValue, SIZE(resolution_info.vRes));
                  nItemsRead=fread(&IntValue, SIZE(IntValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(IntValue);
                  resolution_info.vResUnit=Calculate(IntValue, SIZE(resolution_info.vResUnit));
                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  resolution_info.heightUnit=Calculate(ShortValue, SIZE(resolution_info.heightUnit));
               }break;

               case 1007:
               {
                  mbDisplayInfoFilled=true;

                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  display_info.ColourSpace=Calculate(ShortValue, SIZE(display_info.ColourSpace));

                  for(UInt n=0; n<4; ++n)
                  {
                     nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                     nBytesRead += nItemsRead * SIZE(ShortValue);
                     display_info.Colour[n]=Calculate(ShortValue, SIZE(display_info.Colour[n]));
                  }

                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  display_info.Opacity=Calculate(ShortValue, SIZE(display_info.Opacity));
                  assert (0 <= display_info.Opacity);
                  assert (100 >= display_info.Opacity);

                  Byte c[1];
                  nItemsRead=fread(&c, SIZE(c), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(c);
                  (1 == Calculate(c, SIZE(c))) ? display_info.kind=true : display_info.kind=false;

                  nItemsRead=fread(&c, SIZE(c), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(c);
                  display_info.padding=(UInt)Calculate(c, SIZE(c));
                  assert (0 == display_info.padding);
               }break;

               case 1034:
               {
                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  (1 == Calculate(ShortValue, SIZE(ShortValue))) ? mbCopyright=true : mbCopyright=false;
               }break;

               case 1033:
               case 1036:
               {
                  mbThumbNailFilled=true;

                  nItemsRead=fread(&IntValue, SIZE(IntValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(IntValue);
                  thumbnail.nFormat=Calculate(IntValue, SIZE(thumbnail.nFormat));

                  nItemsRead=fread(&IntValue, SIZE(IntValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(IntValue);
                  thumbnail.nWidth=Calculate(IntValue, SIZE(thumbnail.nWidth));

                  nItemsRead=fread(&IntValue, SIZE(IntValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(IntValue);
                  thumbnail.nHeight=Calculate(IntValue, SIZE(thumbnail.nHeight));

                  nItemsRead=fread(&IntValue, SIZE(IntValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(IntValue);
                  thumbnail.nWidthBytes=Calculate(IntValue, SIZE(thumbnail.nWidthBytes));

                  nItemsRead=fread(&IntValue, SIZE(IntValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(IntValue);
                  thumbnail.nSize=Calculate(IntValue, SIZE(thumbnail.nSize));

                  nItemsRead=fread(&IntValue, SIZE(IntValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(IntValue);
                  thumbnail.nCompressedSize=Calculate(IntValue, SIZE(thumbnail.nCompressedSize));

                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  thumbnail.nBitPerPixel=Calculate(ShortValue, SIZE(thumbnail.nBitPerPixel));

                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  thumbnail.nPlanes=Calculate(ShortValue, SIZE(thumbnail.nPlanes));

                  Int nTotalData=image_resource.nSize - 28; // header
                  Byte *buffer=new Byte[nTotalData];
                  Byte c[1];
                  if(1033 == image_resource.nID)
                  {
                     // In BGR format
                     for(Int n=0; n<nTotalData; n=n +3)
                     {
                        nItemsRead=fread(&c, SIZE(Byte), 1, pFile);
                        nBytesRead += nItemsRead * SIZE(Byte);
                        buffer[n+2]=(Byte)Calculate(c, SIZE(Byte));
                        nItemsRead=fread(&c, SIZE(Byte), 1, pFile);
                        nBytesRead += nItemsRead * SIZE(Byte);
                        buffer[n+1]=(Byte)Calculate(c, SIZE(BYTE));
                        nItemsRead=fread(&c, SIZE(Byte), 1, pFile);
                        nBytesRead += nItemsRead * SIZE(Byte);
                        buffer[n]=(Byte)Calculate(c, SIZE(Byte));
                     }
                  }
                  else if(1036 == image_resource.nID)
                  {
                     // In RGB format
                     for(Int n=0; n<nTotalData; ++n)
                     {
                        nItemsRead=fread(&c, SIZE(BYTE), 1, pFile);
                        nBytesRead += nItemsRead * SIZE(BYTE);
                        buffer[n]=(BYTE)Calculate(c, SIZE(BYTE));
                     }
                  }

                  DeleteN(buffer);
               }break;

               case 1037:
               {
                  nItemsRead=fread(&IntValue, SIZE(IntValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(IntValue);
                  mnGlobalAngle=Calculate(IntValue, SIZE(mnGlobalAngle));
               }break;

               case 1046:
               {
                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  mnColourCount=Calculate(ShortValue, SIZE(ShortValue));
               }break;

               case 1047:
               {
                  nItemsRead=fread(&ShortValue, SIZE(ShortValue), 1, pFile);
                  nBytesRead += nItemsRead * SIZE(ShortValue);
                  mnTransparentIndex=Calculate(ShortValue, SIZE(ShortValue));
               }break;

               default:
               {
                  Byte c[1];
                  for(Int n=0; n<image_resource.nSize; ++n)
                  {
                     nItemsRead=fread(&c, SIZE(c), 1, pFile);
                     nBytesRead += nItemsRead * SIZE(c);
                  }
               }break;
            }
         }
      }
   }
   return nBytesRead==nTotalBytes;
#endif
}
/******************************************************************************/
Bool PSD::ProccessBuffer(Byte *pData, Image &dest)
{
   if(pData)
   {
      Int nHeight=header_info.nHeight,
          nWidth =header_info.nWidth,
          bytesPerPixelPerChannel=header_info.nBitsPerPixel/8,
          nPixels=nWidth*nHeight,
          nTotalBytes=nPixels*bytesPerPixelPerChannel*header_info.nChannels,
          nCounter=0,
          x=0, y=0;
      Dbl max_value=((1<<header_info.nBitsPerPixel)-1);

      switch(header_info.nColourMode)
      {
         case 1: // Grayscale
         case 8: // Duotone
         {
            if(dest.createSoftTry(nWidth, nHeight, 1, (header_info.nBitsPerPixel==32) ? IMAGE_I32 : (header_info.nBitsPerPixel==24) ? IMAGE_I24 : (header_info.nBitsPerPixel==16) ? IMAGE_I16 : IMAGE_L8))
            {
               for(; nCounter<nTotalBytes; nCounter+=header_info.nChannels*bytesPerPixelPerChannel)
               {
                  UInt u=Calculate(pData+nCounter, bytesPerPixelPerChannel); if(header_info.nBitsPerPixel==32)u=Sat(Pow((Flt&)u, 1.0f/1.75f))*0xFFFFFFFFu;
                  dest.pixel(x, y, u);
                  x++; if(x>=nWidth){x=0; y++;}
               }
               return true;
            }
         }break;

         case 2: // Indexed
         {
            // pData holds the indices of loop through the palette and set the correct RGB, 8bpp are supported
            if(colour_mode_data.ColourData && colour_mode_data.nLength==768 && mnColourCount>0)
            if(dest.createSoftTry(nWidth, nHeight, 1, IMAGE_R8G8B8))
            {
               for(; nCounter<nTotalBytes; ++nCounter)
               {
                  UInt index=                      pData[nCounter   ],
                       red  =colour_mode_data.ColourData[index      ],
                       green=colour_mode_data.ColourData[index+  256],
                       blue =colour_mode_data.ColourData[index+2*256];

                  dest.color(x, y, Color(red, green, blue));
                  x++; if(x>=nWidth){x=0; y++;}
               }
               return true;
            }
         }break;

         case 3: // RGB(A), there can be more than 4 channels !!
         {
            if(dest.createSoftTry(nWidth, nHeight, 1, (header_info.nChannels>=4) ? IMAGE_R8G8B8A8 : IMAGE_R8G8B8))
            {
               for(; nCounter<nTotalBytes; nCounter=nCounter + header_info.nChannels * bytesPerPixelPerChannel)
               {
                  UInt red  =                              Calculate(pData+nCounter                          , bytesPerPixelPerChannel)     ,
                       green=                              Calculate(pData+nCounter+  bytesPerPixelPerChannel, bytesPerPixelPerChannel)     ,
                       blue =                              Calculate(pData+nCounter+2*bytesPerPixelPerChannel, bytesPerPixelPerChannel)     ,
                       alpha=((header_info.nChannels>=4) ? Calculate(pData+nCounter+3*bytesPerPixelPerChannel, bytesPerPixelPerChannel) : 0);

                  if(bytesPerPixelPerChannel==4) // Flt
                  {
                     red  =RoundU(Pow(Sat((Flt&)red  ), 1/2.12f)*255);
                     green=RoundU(Pow(Sat((Flt&)green), 1/2.12f)*255);
                     blue =RoundU(Pow(Sat((Flt&)blue ), 1/2.12f)*255);
                  }else
                  REP(bytesPerPixelPerChannel-1){red>>=8; green>>=8; blue>>=8; alpha>>=8;}
                  dest.color(x, y, Color(red, green, blue, alpha));
                  x++; if(x>=nWidth){x=0; y++;}
               }
               return true;
            }
         }break;

         case 4: // CMYK
         {
            if(dest.createSoftTry(nWidth, nHeight, 1, IMAGE_R8G8B8))
            {
               for(; nCounter<nTotalBytes; nCounter+=header_info.nChannels*bytesPerPixelPerChannel)
               {
                  Dbl c=1-Calculate(pData+nCounter                          , bytesPerPixelPerChannel)/max_value,
                      m=1-Calculate(pData+nCounter  +bytesPerPixelPerChannel, bytesPerPixelPerChannel)/max_value,
                      Y=1-Calculate(pData+nCounter+2*bytesPerPixelPerChannel, bytesPerPixelPerChannel)/max_value,
                      k=1-Calculate(pData+nCounter+3*bytesPerPixelPerChannel, bytesPerPixelPerChannel)/max_value;

                  Int red, green, blue; CMYKToRGB(c, m, Y, k, red, green, blue);
                  dest.color(x, y, Color(red, green, blue));
                  x++; if(x>=nWidth){x=0; y++;}
               }
               return true;
            }
         }break;

         case 7: // Multichannel
         {
            if(header_info.nChannels==1) // for now support just one channel
            if(dest.createSoftTry(nWidth, nHeight, 1, IMAGE_L8))
            {
               for(; nCounter<nTotalBytes; nCounter+=header_info.nChannels*bytesPerPixelPerChannel)
               {
                  Dbl a=Calculate(pData+nCounter, bytesPerPixelPerChannel)/max_value;
                  dest.pixel(x, y, RoundU(a*255));
                  x++; if(x>=nWidth){x=0; y++;}
               }
               return true;
            }
         }break;

         case 9: // LAB
         {
            if(dest.createSoftTry(nWidth, nHeight, 1, (header_info.nChannels==4) ? IMAGE_R8G8B8A8 : IMAGE_R8G8B8))
            {
               Dbl L_coef=max_value/100.0,
                   a_coef=max_value/256.0,
                   b_coef=max_value/256.0;
               for(; nCounter<nTotalBytes; nCounter+=header_info.nChannels*bytesPerPixelPerChannel)
               {
                  Dbl  L    =                              Calculate(pData+nCounter                          , bytesPerPixelPerChannel)/L_coef         ,
                       A    =                              Calculate(pData+nCounter+  bytesPerPixelPerChannel, bytesPerPixelPerChannel)/a_coef-128     ,
                       B    =                              Calculate(pData+nCounter+2*bytesPerPixelPerChannel, bytesPerPixelPerChannel)/b_coef-128     ;
                  UInt alpha=((header_info.nChannels==4) ? Calculate(pData+nCounter+3*bytesPerPixelPerChannel, bytesPerPixelPerChannel)            : 0);

                  Int red, green, blue; LABToRGB(L, A, B, red, green, blue); REP(bytesPerPixelPerChannel-1)alpha>>=8;
                  dest.color(x, y, Color(red, green, blue, alpha));
                  x++; if(x>=nWidth){x=0; y++;}
               }
               return true;
            }
         }break;
      }
   }
   return false;
}
/******************************************************************************/
Bool PSD::ReadImageData(File &f, Image &dest)
{
   Bool ok=false;
   switch(GetU16(f))
   {
      case 0:	// raw data
      {
         Int nWidth=header_info.nWidth;
         Int nHeight=header_info.nHeight;
         Int bytesPerPixelPerChannel=header_info.nBitsPerPixel/8;

         Int nPixels=nWidth*nHeight;
         Int nTotalBytes=nPixels*bytesPerPixelPerChannel*header_info.nChannels;
         Int nBytesRead=0;

         Byte *pData=null;

         switch(header_info.nColourMode)
         {
            case 1: // Grayscale
            case 7: // Multichannel
            case 8: // Duotone
            {
               pData=new Byte[nTotalBytes];
               nBytesRead=f.getReturnSize(pData, nTotalBytes);
            }break;

            case 2: // Indexed
            {
               if(mnColourCount>0 && colour_mode_data.ColourData!=0)
               {
                  pData=new Byte[nTotalBytes];
                  nBytesRead=f.getReturnSize(pData, nTotalBytes);
               }
            }break;

            case 3: // RGB
            case 4: // CMYK
            case 9: // LAB
            {
               pData=new Byte[nTotalBytes];
               for(Int nColour=0; nColour<header_info.nChannels  ; ++nColour)
               for(Int nPos   =0; nPos   <nPixels                ; ++nPos   )
               for(Int nByte  =0; nByte  <bytesPerPixelPerChannel; ++nByte  )
               {
                  pData[nPos*header_info.nChannels*bytesPerPixelPerChannel + nColour*bytesPerPixelPerChannel + nByte]=f.getByte();
                  nBytesRead++;
               }
            }break;
         }

         if(nBytesRead==nTotalBytes)ok=ProccessBuffer(pData, dest);
         DeleteN(pData);
      }break;

      case 1:	// RLE compression
      {
         Int nWidth=header_info.nWidth;
         Int nHeight=header_info.nHeight;
         Int bytesPerPixelPerChannel=header_info.nBitsPerPixel/8;

         Int nPixels=nWidth*nHeight;
         Int nTotalBytes=nPixels*bytesPerPixelPerChannel*header_info.nChannels;

         Byte *pData=new Byte[nTotalBytes];
         Byte *p=pData;

         // The RLE-compressed data is preceeded by a 2-byte data count for each row in the data, which we're going to just skip
         f.skip(nHeight*header_info.nChannels*2);

         for(Int channel=0; channel<header_info.nChannels; channel++)
         {
            // Read the RLE data.
            Int   count=0;
            while(count<nPixels)
            {
               Int len=f.getByte();
               if( len<128)
               {
                  len++;
                  count+=len;
                  while(len)
                  {
                     *p++=f.getByte();
                     len--;
                  }
               }else
               if(len>128)
               {
                  // Next -len+1 bytes in the dest are replicated from next source byte, Interpret len as a negative 8-bit Int
                  len^=0xFF;
                  len+=2;
                  Byte value=f.getByte();
                  count+=len;
                  while(len)
                  {
                     *p++=value;
                     len--;
                  }
               }/*else
               if(len==128)
               {
                  // Do nothing
               }*/
            }
         }

         Byte *pSrc =pData,
              *pDest=new Byte[nTotalBytes];

         Int nPixelCounter=0;
         for(Int nColour=0; nColour<header_info.nChannels; ++nColour)
         {
            nPixelCounter=nColour*bytesPerPixelPerChannel;
            for(Int nPos=0; nPos<nPixels; nPos++)
            {
               CopyFast(pDest+nPixelCounter, pSrc, bytesPerPixelPerChannel);
               pSrc++;

               nPixelCounter+=header_info.nChannels*bytesPerPixelPerChannel;
            }
         }
         ok=ProccessBuffer(pDest, dest);
         DeleteN(pData);
         DeleteN(pDest);
      }break;

      case  2: break; // ZIP without prediction, no specification
      case  3: break; // ZIP with    prediction, no specification
      default: break; // unknown format
   }

   return ok;
}
/******************************************************************************/
static Color UnblendWhite(Color c)
{
   // original*alpha + white*(1-alpha) = c
   // original = (c - white*(1-alpha))/alpha
   if(c.a)
   {
      Int a_2=c.a/2, a1=255-c.a;
      c.r=Mid((255*(c.r-a1)+a_2)/c.a, 0, 255);
      c.g=Mid((255*(c.g-a1)+a_2)/c.a, 0, 255);
      c.b=Mid((255*(c.b-a1)+a_2)/c.a, 0, 255);
   }
   return c;
}
static Color Unblend(Color c, C Color &back)
{
   // original*alpha + back*(1-alpha) = c
   // original = (c - back*(1-alpha))/alpha
   // original = (c - back + back*alpha)/alpha
   // original = (c - back)/alpha + back
   if(c.a)
   {
      Int a_2=c.a/2;
      c.r=Mid((255*(c.r-back.r)+a_2)/c.a+back.r, 0, 255);
      c.g=Mid((255*(c.g-back.g)+a_2)/c.a+back.g, 0, 255);
      c.b=Mid((255*(c.b-back.b)+a_2)/c.a+back.b, 0, 255);
   }
   return c;
}
#endif
/******************************************************************************/
Bool Image::ImportPSD(File &f)
{
   del();

#if SUPPORT_PSD
   PSD psd;
   if(psd.ReadHeader                 (f, psd.header_info))
   if(psd.ReadColourModeData         (f, psd.colour_mode_data))
   if(psd.ReadImageResource          (f, psd.image_resource))
   if(psd.ReadLayerAndMaskInfoSection(f))
   if(psd.ReadImageData              (f, T))
   {
      if(psd.merged_alpha && ImageTI[hwType()].a) // PSD's are blended on white background
      {
         REPD(y, h())
         REPD(x, w())color(x, y, UnblendWhite(color(x, y)));
         transparentToNeighbor();
      }
      return true;
   }
#endif
   return false;
}
Bool Image::ImportPSD(C Str &name)
{
#if SUPPORT_PSD
   File f; if(f.readTry(name))return ImportPSD(f);
#endif
   del(); return false;
}
/******************************************************************************/
}
/******************************************************************************/
