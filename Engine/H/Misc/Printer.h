/******************************************************************************/
struct PrinterInfo
{
   Str   name, model;
   Bool  color, // if can print in color
         local; // if available locally (if false then network)
   Int   width, height, // paper size, in 1/10 of mili-meter units
         dpi;
   VecI2 res; // get resolution (in pixels)

   Dbl  widthMM()C {return  width/10.0;} // get width  in mili-meters
   Dbl heightMM()C {return height/10.0;} // get height in mili-meters

   Dbl  widthCM()C {return  width/100.0;} // get width  in centi-meters
   Dbl heightCM()C {return height/100.0;} // get height in centi-meters

   Dbl  widthM()C {return  width/10000.0;} // get width  in meters
   Dbl heightM()C {return height/10000.0;} // get height in meters
};
Bool GetPrinters(MemPtr<PrinterInfo> printers); // get list of available printers, false on fail
/******************************************************************************/
struct RawPrinter
{
   // manage
   Bool    connect(C Str &printer_name); // connect to 'printer_name' printer, false on fail
   void disconnect();

  ~RawPrinter() {disconnect();}
   RawPrinter();

#if !EE_PRIVATE
private:
#endif
   Str8 data;
   Str  printer_name;
#if WINDOWS_OLD
#if EE_PRIVATE
	HANDLE printer;
#else
   Ptr  printer;
#endif
#endif
#if EE_PRIVATE
   Bool send(C Str8 &data, C Str &document_name=S);
#endif
};
STRUCT_PRIVATE(ReceiptPrinter , RawPrinter)
//{
   // manage
   Bool    connect(C Str &printer_name); // connect to 'printer_name' printer, false on fail
   void disconnect();

   // print
   void operator++(); // skip line
   void operator+=(C Str8 &text); // print text
   Bool operator+=(C Image &img); // print image, false on fail

   void justify(Int j); // set justification, <0=left, 0=center, >0 right

   void barcodeWidth ( Byte width ); // set   bar-code width
   void barcodeHeight( Byte height); // set   bar-code height
   void barcode39    (C Str8 &code); // print bar-code based on specified 'code' using "Code 39"
   void barcode93    (C Str8 &code); // print bar-code based on specified 'code' using "Code 93"
   void barcode128   (C Str8 &code); // print bar-code based on specified 'code' using "Code 128"

   Bool end(C Str &document_name=S); // flush all queued printing commands to printer and cut paper, false on fail

   // cash drawer
   Bool openCashDrawer(); // open cash drawer (this does not require 'end'), false on fail

#if !EE_PRIVATE
private:
#endif
   Byte code_page;
#if EE_PRIVATE
   void init();
   void codePage(Byte cp);
   void lineHeight(Byte height); // set custom  line height
   void lineHeight(           ); // set default line height
#endif
};
STRUCT_PRIVATE(LabelPrinter , RawPrinter)
//{
   // manage
   Bool    connect(C Str &printer_name); // connect to 'printer_name' printer, false on fail
   void disconnect();

   // print
   void text (C VecI2 &pos, C Str  &text); // print text  at 'pos' position
   Bool image(C VecI2 &pos, C Image &img); // print image at 'pos' position, false on fail

   void barcodeHeight(Byte height); // set bar-code height
   void barcode39    (C VecI2 &pos, C Str8 &code); // print bar-code based on specified 'code' at 'pos' position using "Code 39"
   void barcode93    (C VecI2 &pos, C Str8 &code); // print bar-code based on specified 'code' at 'pos' position using "Code 93"
   void barcode128   (C VecI2 &pos, C Str8 &code); // print bar-code based on specified 'code' at 'pos' position using "Code 128"

   void speed(Int speed); // set printing speed, in inches per second (2..12)

   Bool end(C Str &document_name=S); // flush all queued printing commands to printer, false on fail

#if !EE_PRIVATE
private:
#endif
#if EE_PRIVATE
   void init();
   void pos(C VecI2 &pos); // set print position
#endif
   VecI2 res;
};
/******************************************************************************/
void Barcode128(C Str8 &barcode, MemPtr<Bool> data); // generate "Code 128" bar-code bitmap
/******************************************************************************/
