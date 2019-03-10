/******************************************************************************/
#ifndef ETC_H
#define ETC_H
/******************************************************************************/
void DecompressBlockETC1  (C Byte *b, Color (&block)[4][4]);
void DecompressBlockETC2  (C Byte *b, Color (&block)[4][4]);
void DecompressBlockETC2A1(C Byte *b, Color (&block)[4][4]);
void DecompressBlockETC2A8(C Byte *b, Color (&block)[4][4]);

void DecompressBlockETC1  (C Byte *b, Color *dest, Int pitch);
void DecompressBlockETC2  (C Byte *b, Color *dest, Int pitch);
void DecompressBlockETC2A1(C Byte *b, Color *dest, Int pitch);
void DecompressBlockETC2A8(C Byte *b, Color *dest, Int pitch);

Color DecompressPixelETC1  (C Byte *b, Int x, Int y);
Color DecompressPixelETC2  (C Byte *b, Int x, Int y);
Color DecompressPixelETC2A1(C Byte *b, Int x, Int y);
Color DecompressPixelETC2A8(C Byte *b, Int x, Int y);
/******************************************************************************/
#endif
/******************************************************************************/
