/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
Bool GetPrinters(MemPtr<PrinterInfo> printers)
{
   Bool ok=false;
   printers.clear();
#if WINDOWS_OLD
   const DWORD level=2; // this is matched with PRINTER_INFO_2 used below
   DWORD num=0, size=0;
   EnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS, null, level, null, 0, &size, &num);
   Memt<Byte> temp;
again:
   temp.setNum(size);
   PRINTER_INFO_2 *list=(PRINTER_INFO_2*)temp.data();
   if(EnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS, null, level, (LPBYTE)list, temp.elms(), &size, &num))
   {
      ok=true;
      printers.setNum(num); FREPA(printers)
      {
       C PRINTER_INFO_2 &src=list[i];
         PrinterInfo    &pi =printers[i];
         pi.name  =         src.pPrinterName;
         pi.model =         src.pDriverName;
         pi.color =        (src.pDevMode->dmColor==DMCOLOR_COLOR);
         pi.local =FlagTest(src.Attributes, PRINTER_ATTRIBUTE_LOCAL);
         pi.width =         src.pDevMode->dmPaperWidth;
         pi.height=         src.pDevMode->dmPaperLength;
         pi.dpi   =         src.pDevMode->dmPrintQuality;
         pi.res.set(pi.width, pi.height)*=pi.dpi; pi.res/=254; // 1 inch = 25.4 mm
      }
   }else
   if(Int(size)>temp.elms())goto again;
#endif
   return ok;
}
/******************************************************************************/
// RAW PRINTER
/******************************************************************************/
RawPrinter::RawPrinter()
{
#if WINDOWS_OLD
   printer=null;
#endif
}
void RawPrinter::disconnect()
{
#if WINDOWS_OLD
	if(printer){ClosePrinter(printer); printer=null;}
   printer_name.del();
#endif
}
Bool RawPrinter::connect(C Str &printer_name)
{
   disconnect();
#if WINDOWS_OLD
	if(OpenPrinterW(ConstCast(WChar(printer_name())), &printer, null))
#else
   if(0)
#endif
   {
      T.printer_name=printer_name; return true;
   }
   return false;
}
/******************************************************************************/
Bool RawPrinter::send(C Str8 &data, C Str &document_name)
{
   Bool ok=false;
#if WINDOWS_OLD
   if(printer)
   {
	   DOC_INFO_1W doc_info;
	   doc_info.pDocName   =ConstCast(WChar(document_name.is() ? document_name() : App.name().is() ? App.name()() : u"Doc")); // some printers may fail if no name is specified
	   doc_info.pOutputFile=null;
	   doc_info.pDatatype  =L"RAW";
	   if(DWORD job_id=StartDocPrinterW(printer, 1, (LPBYTE)&doc_info))
      {
	      if(StartPagePrinter(printer))
	      {
	         DWORD written;
	         if(WritePrinter(printer, (Ptr)data(), data.length(), &written))
               ok=(written==data.length());
		      EndPagePrinter(printer);
         }
		   EndDocPrinter(printer);
         /*HANDLE printer_job=null; if(OpenPrinterW(ConstCast(WChar((printer_name+", Job "+(UInt)job_id)())), &printer_job, null))
         {
            Byte buffer[65536]; DWORD dwBytesRead=0;
            if(ReadPrinter(printer_job, buffer, SIZE(buffer), &dwBytesRead) && dwBytesRead)
            {
               int z=0;
            }
            ClosePrinter(printer_job);
         }*/
      }
   }
#endif
   return ok;
}
/******************************************************************************/
// RECEIPT PRINTER
/******************************************************************************/
void ReceiptPrinter::disconnect() {super::disconnect();}
Bool ReceiptPrinter::   connect(C Str &printer_name)
{
   if(super::connect(printer_name))
   {
      init(); return true;
   }
   return false;
}
void ReceiptPrinter::init()
{
   code_page=0;
   data="\x1B@"; // reset state
   barcodeHeight(40); // set default bar-code height (without this, printing bar-codes will fail)
}
Bool ReceiptPrinter::end(C Str &document_name)
{
   data+="\x1DVA\x03";
   Bool ok=send(data, document_name);
   init();
   return ok;
}
/******************************************************************************/
void ReceiptPrinter::lineHeight(Byte height)
{
   data+="\x1B"; data+='3'; data.alwaysAppend(height);
}
void ReceiptPrinter::lineHeight()
{
   data+="\x1B"; data+='2'; // default
}
/******************************************************************************/
void ReceiptPrinter::codePage(Byte cp)
{
   if(code_page!=cp)
   {
      code_page=cp;
   #if 1
      data+="\x1Bt";
      data.alwaysAppend(cp);
   #else
      data+="\x1C\x7D\x26";
      data.alwaysAppend(cp&0xFF);
      data.alwaysAppend(cp>>8);
   #endif
   }
}
/******************************************************************************/
void ReceiptPrinter::operator++()
{
   data+='\n';
}
void ReceiptPrinter::operator+=(C Str8 &text)
{
   if(text.is())
   {
   #if 1
      data+=text;
   #else // writing unicode requires setting code pages, however there's no universal standard, as it depends on printer model
      FREPA(text)
      {
         Char c=text[i];
         if(!HasUnicode(c))
         {
            codePage(0);
            data+=c;
         }else
         {
            codePage(27);
            data+=MultiByte(874, c);
         }
      }
   #endif
      data+='\n';
   }
}
/******************************************************************************/
void ReceiptPrinter::justify(Int j)
{
   data+="\x1B\x61"; data.alwaysAppend(Sign(j)+1); // -1..1 -> 0..2
}
/******************************************************************************/
// BAR CODE
/******************************************************************************/
void ReceiptPrinter::barcodeWidth(Byte width)
{
   data+="\x1D\x77"; data.alwaysAppend(width);
}
void ReceiptPrinter::barcodeHeight(Byte height)
{
   data+="\x1D\x68"; data.alwaysAppend(height);
}
void ReceiptPrinter::barcode39(C Str8 &code)
{
   if(code.is())
   {
      data+="\x1D\x6B\x04";
      data+=code;
      data.alwaysAppend('\0');
    //data+='\n'; this shouldn't be done as barcodes automatically end line
   }
}
void ReceiptPrinter::barcode93(C Str8 &code)
{
   if(code.is())
   {
      data+="\x1D\x6B\x07";
      data+=code;
      data.alwaysAppend('\0');
    //data+='\n'; this shouldn't be done as barcodes automatically end line
   }
}
void ReceiptPrinter::barcode128(C Str8 &code)
{
   if(code.is())
   {
      data+="\x1D\x6B\x08";
      data+=code;
      data.alwaysAppend('\0');
    //data+='\n'; this shouldn't be done as barcodes automatically end line
   }
}
/******************************************************************************/
// IMAGE
/******************************************************************************/
static inline Bool ColToBit(C Vec4 &c)
{
   return (1-SRGBLumOfSRGBColor(c.xyz))*c.w>=0.5f;
}
Bool ReceiptPrinter::operator+=(C Image &img)
{
   Image tmp; C Image *src=&img; if(img.compressed())if(img.copyTry(tmp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&tmp;else return false;
   if(src->lockRead())
   {
   #if 1
      Int w=DivCeil8(src->lw()), h=src->lh();
      data+="\x1D\x76\x30"; data.alwaysAppend(0);
      data.alwaysAppend(w&0xFF);
      data.alwaysAppend(w>>8  );
      data.alwaysAppend(h&0xFF);
      data.alwaysAppend(h>>8  );
      FREPD(y, h)
      for(Int x=0; x<src->lw(); )
      {
         Byte b=0; REP(8)
         {
            if(ColToBit(src->colorF(x, y)))b|=(1<<i);
            x++;
         }
         data.alwaysAppend(b);
      }
   #else // this mode needs high DPI with 24 lines to achieve highest possible DPI output matching method above, however it results in padding to 24 lines, so don't use it
      lineHeight(0); // have to set zero line height to avoid any padding between line chunks
      for(Int y=0; y<src->lh(); y+=24)
      {
         data+="\x1B\x2A"; data.alwaysAppend(33);
         data.alwaysAppend(src->lw()&0xFF);
         data.alwaysAppend(src->lw()>>8  );
         FREPD(x, src->lw())
         FREPD(sy, 3)
         {
            Byte b=0; FREP(8)if(ColToBit(src->colorF(x, y+7-i + sy*8)))b|=(1<<i);
            data.alwaysAppend(b);
         }
         data+='\n';
      }
      lineHeight();
   #endif
      src->unlock();
      return true;
   }
   return false;
}
/******************************************************************************/
Bool ReceiptPrinter::openCashDrawer()
{
   const Int pin=0, on_ms=120, off_ms=240;
   Str8 data="\x1Bp";
   data.alwaysAppend(48+pin);
   data.alwaysAppend( on_ms/2);
   data.alwaysAppend(off_ms/2);
   return send(data);
}
/******************************************************************************
void ReceiptPrinter::test()
{
   if(printer)
   {
      FREP(256)
      {
         //Str8 data="\x1D\x49";
         //Str8 data="\x1D\x72";
         Str8 data="\x10\x04";
         data.alwaysAppend(i);
         send(data);
      }
      int z=0;
   }
}
/******************************************************************************/
// LABEL PRINTER
/******************************************************************************/
static Str8 ZebraText(C Str8 &text)
{
   Str8 out; out.reserve(4+3+text.length()+3); // 4="^FH`" 3="^FD" 3="^FS"
   out="^FH`^FD";
   FREPA(text)
   {
      Char8 c=text[i]; Byte b=c; if(b<32 || b>=128 || c=='`' || c=='^')
      {
         out+='`';
         out+=Digits16[b>>4];
         out+=Digits16[b&15];
      }else
      {
         out+=c;
      }
   }
   out+="^FS";
   return out;
}
void LabelPrinter::disconnect() {super::disconnect();}
Bool LabelPrinter::   connect(C Str &printer_name)
{
   Memc<PrinterInfo> pis; GetPrinters(pis); REPA(pis)
   {
    C PrinterInfo &pi=pis[i]; if(pi.name==printer_name)
      {
         res=pi.res; goto connect;
      }
   }
   disconnect(); return false;
connect:
   if(super::connect(printer_name))
   {
      init(); return true;
   }
   res.zero(); return false;
}
void LabelPrinter::init()
{
   data ="^XA"; // start
   data+="^CI28"; // code page UTF-8
   data+="^MUD"; // set units to Dots
   data+="^LH0,0"; // label home position (origin point)
   data+="^JMA"; // set full DPI print resolution
   data+="~SD15"; // set medium darkness 0..30
   data+="^PW"; data+=res.x; // set print width (in dots)
   data+="^LL"; data+=res.y; // label length (in dots)

 //data+="^MNN"; //     continuous media
 //data+="^MNM"; // non-continuous media

   /* media type
   data+="^MTT"; // Thermal Transfer Media - Ribbon
   data+="^MTD"; // Direct Thermal Media - No Ribbon*/

 //data+="^CC`"; // replace ` to be command character instead of ^, this is not working, instead 'ZebraText' is used
}
Bool LabelPrinter::end(C Str &document_name)
{
   data+="^XZ"; // finish
   Bool ok=send(data, document_name);
   init();
   return ok;
}
/******************************************************************************/
void LabelPrinter::pos(C VecI2 &pos)
{
   data+="^FO";
   data+=pos.x;
   data+=',';
   data+=pos.y;
}
void LabelPrinter::text(C VecI2 &pos, C Str &text)
{
   if(text.is())
   {
      T.pos(pos);
      data+="^ADN,18,10";
      data+=ZebraText(UTF8(text));
   }
}
Bool LabelPrinter::image(C VecI2 &pos, C Image &img)
{
   Image tmp; C Image *src=&img; if(img.compressed())if(img.copyTry(tmp, -1, -1, -1, IMAGE_R8G8B8A8, IMAGE_SOFT, 1))src=&tmp;else return false;
   if(src->lockRead())
   {
      T.pos(pos);
      Int w=DivCeil8(src->lw()), h=src->lh();
      data+="^GFB,"; // binary
      data+=w*h; data+=',';
      data+=w*h; data+=',';
      data+=w  ; data+=','; // width
      FREPD(y, h)
      for(Int x=0; x<src->lw(); )
      {
         Byte b=0; REP(8)
         {
            if(ColToBit(src->colorF(x, y)))b|=(1<<i);
            x++;
         }
         data.alwaysAppend(b);
      }
      src->unlock();
      return true;
   }
   return false;
}
void LabelPrinter::barcodeHeight(Byte height)
{
   data+=S8+"^BY2,3.0,"+height;
}
void LabelPrinter::barcode39(C VecI2 &pos, C Str8 &code)
{
   if(code.is())
   {
      T.pos(pos);
      data+="^B3";
      data+=ZebraText(code);
   }
}
void LabelPrinter::barcode93(C VecI2 &pos, C Str8 &code)
{
   if(code.is())
   {
      T.pos(pos);
      data+="^BA";
      data+=ZebraText(code);
   }
}
void LabelPrinter::barcode128(C VecI2 &pos, C Str8 &code)
{
   if(code.is())
   {
      T.pos(pos);
      data+="^BC";
      data+=ZebraText(code);
   }
}
/******************************************************************************/
void LabelPrinter::speed(Int speed)
{
   data+="^PR";
   data+=Mid(speed, 2, 12); // print speed (inch per second)
}
/******************************************************************************/
namespace Code128
{

static const U16 Pattern[]=
{
       // index: pattern    , widths
   1740, //   0: 11011001100, 212222
   1644, //   1: 11001101100, 222122
   1638, //   2: 11001100110, 222221
   1176, //   3: 10010011000, 121223
   1164, //   4: 10010001100, 121322
   1100, //   5: 10001001100, 131222
   1224, //   6: 10011001000, 122213
   1220, //   7: 10011000100, 122312
   1124, //   8: 10001100100, 132212
   1608, //   9: 11001001000, 221213
   1604, //  10: 11001000100, 221312
   1572, //  11: 11000100100, 231212
   1436, //  12: 10110011100, 112232
   1244, //  13: 10011011100, 122132
   1230, //  14: 10011001110, 122231
   1484, //  15: 10111001100, 113222
   1260, //  16: 10011101100, 123122
   1254, //  17: 10011100110, 123221
   1650, //  18: 11001110010, 223211
   1628, //  19: 11001011100, 221132
   1614, //  20: 11001001110, 221231
   1764, //  21: 11011100100, 213212
   1652, //  22: 11001110100, 223112
   1902, //  23: 11101101110, 312131
   1868, //  24: 11101001100, 311222
   1836, //  25: 11100101100, 321122
   1830, //  26: 11100100110, 321221
   1892, //  27: 11101100100, 312212
   1844, //  28: 11100110100, 322112
   1842, //  29: 11100110010, 322211
   1752, //  30: 11011011000, 212123
   1734, //  31: 11011000110, 212321
   1590, //  32: 11000110110, 232121
   1304, //  33: 10100011000, 111323
   1112, //  34: 10001011000, 131123
   1094, //  35: 10001000110, 131321
   1416, //  36: 10110001000, 112313
   1128, //  37: 10001101000, 132113
   1122, //  38: 10001100010, 132311
   1672, //  39: 11010001000, 211313
   1576, //  40: 11000101000, 231113
   1570, //  41: 11000100010, 231311
   1464, //  42: 10110111000, 112133
   1422, //  43: 10110001110, 112331
   1134, //  44: 10001101110, 132131
   1496, //  45: 10111011000, 113123
   1478, //  46: 10111000110, 113321
   1142, //  47: 10001110110, 133121
   1910, //  48: 11101110110, 313121
   1678, //  49: 11010001110, 211331
   1582, //  50: 11000101110, 231131
   1768, //  51: 11011101000, 213113
   1762, //  52: 11011100010, 213311
   1774, //  53: 11011101110, 213131
   1880, //  54: 11101011000, 311123
   1862, //  55: 11101000110, 311321
   1814, //  56: 11100010110, 331121
   1896, //  57: 11101101000, 312113
   1890, //  58: 11101100010, 312311
   1818, //  59: 11100011010, 332111
   1914, //  60: 11101111010, 314111
   1602, //  61: 11001000010, 221411
   1930, //  62: 11110001010, 431111
   1328, //  63: 10100110000, 111224
   1292, //  64: 10100001100, 111422
   1200, //  65: 10010110000, 121124
   1158, //  66: 10010000110, 121421
   1068, //  67: 10000101100, 141122
   1062, //  68: 10000100110, 141221
   1424, //  69: 10110010000, 112214
   1412, //  70: 10110000100, 112412
   1232, //  71: 10011010000, 122114
   1218, //  72: 10011000010, 122411
   1076, //  73: 10000110100, 142112
   1074, //  74: 10000110010, 142211
   1554, //  75: 11000010010, 241211
   1616, //  76: 11001010000, 221114
   1978, //  77: 11110111010, 413111
   1556, //  78: 11000010100, 241112
   1146, //  79: 10001111010, 134111
   1340, //  80: 10100111100, 111242
   1212, //  81: 10010111100, 121142
   1182, //  82: 10010011110, 121241
   1508, //  83: 10111100100, 114212
   1268, //  84: 10011110100, 124112
   1266, //  85: 10011110010, 124211
   1956, //  86: 11110100100, 411212
   1940, //  87: 11110010100, 421112
   1938, //  88: 11110010010, 421211
   1758, //  89: 11011011110, 212141
   1782, //  90: 11011110110, 214121
   1974, //  91: 11110110110, 412121
   1400, //  92: 10101111000, 111143
   1310, //  93: 10100011110, 111341
   1118, //  94: 10001011110, 131141
   1512, //  95: 10111101000, 114113
   1506, //  96: 10111100010, 114311
   1960, //  97: 11110101000, 411113
   1954, //  98: 11110100010, 411311
   1502, //  99: 10111011110, 113141
   1518, // 100: 10111101110, 114131
   1886, // 101: 11101011110, 311141
   1966, // 102: 11110101110, 411131
   1668, // 103: 11010000100, 211412
   1680, // 104: 11010010000, 211214
   1692, // 105: 11010011100, 211232
},
   StopPattern=6379; // 1100011101011, 2331112

enum
{
   QUIET_ZONE_LEN=10,
   CHAR_LEN      =11,
   STOP_LEN      =13,
};
enum MODE : Byte
{
   MODE_A,
   MODE_B,
   MODE_C,
};

static Byte SwitchCode(Int from, MODE to)
{
   switch(from)
   {
      default: switch(to) // start
      {
         case MODE_A: return 103;
         case MODE_B: return 104;
         case MODE_C: return 105;
      }break;

      case MODE_A: switch(to)
      {
         case MODE_B: return 100;
         case MODE_C: return 99;
      }break;

      case MODE_B: switch(to)
      {
         case MODE_A: return 101;
         case MODE_C: return 99;
      }break;

      case MODE_C: switch(to)
      {
        case MODE_B: return 100;
        case MODE_A: return 101;
      }break;
   }
   return -1;
}

static Int CharToCodeA(Char8 value)
{
   if(value>=' ' && value<='_')return value-' ';
   if(value>=0   && value< ' ')return value+64;
 /*if(value==FNC1)return 102;
   if(value==FNC2)return  97;
   if(value==FNC3)return  96;
   if(value==FNC4)return 101;*/
   return -1;
}
static Int CharToCodeB(Char8 value)
{
   if(value>=32)return value-32;
 /*if(value==FNC1)return 102;
   if(value==FNC2)return  97;
   if(value==FNC3)return  96;
   if(value==FNC4)return 100;*/
   return -1;
}
static Int CharToCodeC(Char8 a, Char8 b)
{
   if(a>='0' && a<='9'
   && b>='0' && b<='9')return (a-'0')*10+(b-'0');
   return -1;
}

static void SetBit(Memt<Byte, 65536/3> &bits, Int bit, Bool value)
{
   FlagSet(bits[bit>>3], 1<<(bit&7), value);
}
static Bool GetBit(Memt<Byte, 65536/3> &bits, Int bit)
{
   return FlagTest(bits[bit>>3], 1<<(bit&7));
}

struct Best
{
   Int                 index, length, best_length;
 C Str8               &barcode;
   Memt<Byte, 65536/3> cur_c, best_c;
   
   void find() // for simplicity this checks only MODE_B and MODE_C (ignoring MODE_A)
   {
      if(!InRange(index, barcode))
      {
       //if(length<best_length) already checked before calling 'find'
         {
            best_length=length;
            best_c     =cur_c; // copy best MODE map
         }
      }else
      {
         Char8 c=barcode[index];
         Int   last_mode_c=((index>=1) ? GetBit(cur_c, index-1) : -1); // if at the start, then set -1 mode (none) to force mode change
         REPD(mode_c, 2)
         {
            Int code=(mode_c ? CharToCodeC(c, barcode[index+1]) : CharToCodeB(c)); if(code>=0) // if character can be encoded in this mode
            {
               Byte d_length=(last_mode_c!=mode_c)+1; // changing modes requires 1 extra character
               length+=d_length;
               if(length<best_length)
               {
                            SetBit(cur_c, index  , mode_c);
                  if(mode_c)SetBit(cur_c, index+1, mode_c); // for mode C we're going to advance 2 characters, so set mode for 2nd too
                  Byte d_index=(mode_c ? 2 : 1); // MODE_C consumes 2 characters
                  index+=d_index;
                  find();
                  index-=d_index;
               }
               length-=d_length;
            }
         }
      }
   }

   Best(C Str8 &barcode) : barcode(barcode)
   {
      index=length=0; best_length=INT_MAX;
      Int bytes=DivCeil8(barcode.length());
       cur_c.setNum(bytes);
      best_c.setNum(bytes);
      find();
   }
};

static void Append(MemPtr<Bool> &data, Int &pos, UInt value, Int length)
{
   REP(length)data[pos++]=FlagTest(value, 1<<i);
}

} // namespace Code128

void Barcode128(C Str8 &barcode, MemPtr<Bool> data)
{
   using namespace Code128;
   Best best(barcode);
   if(barcode.is() && best.best_length<INT_MAX)
   {
      data.setNum(best.best_length*CHAR_LEN + CHAR_LEN + STOP_LEN); // data+hash+stop
      Int  last_mode_c=-1, pos=0, hash_pos=0;
      UInt hash=0;
      for(Int i=0; i<barcode.length(); i++)
      {
         MODE mode=(GetBit(best.best_c, i) ? MODE_C : MODE_B);
         if(last_mode_c!=mode)
         {
            Byte code=SwitchCode(last_mode_c, mode);
            Append(data, pos, Pattern[code], CHAR_LEN);
            hash+=Max(1, hash_pos++)*code;
            last_mode_c=mode;
         }
         Byte  code;
         Char8 c=barcode[i];
         switch(mode)
         {
            case MODE_A: code=CharToCodeA(c); break;
            case MODE_B: code=CharToCodeB(c); break;
            default    : code=CharToCodeC(c, barcode[++i]); break; // MODE_C
         }
         Append(data, pos, Pattern[code], CHAR_LEN);
         hash+=(hash_pos++)*code;
      }
      // hash
      Append(data, pos, Pattern[hash%103], CHAR_LEN);
      // stop
      Append(data, pos, StopPattern, STOP_LEN);
   }else data.clear();
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
