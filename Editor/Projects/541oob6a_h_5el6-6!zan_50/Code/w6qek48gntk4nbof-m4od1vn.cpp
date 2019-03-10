/******************************************************************************

   This tutorial shows the basics of compression using 'Compress' and 'Decompress'

   Please note that it may take up to 1 minute until the tutorial starts,
      because of the big source size used for compression testing.

/******************************************************************************/
File src,

    snappy_compressed,
       lz4_compressed,
      zlib_compressed,
      zstd_compressed,
      lzma_compressed,

    snappy_decompressed,
       lz4_decompressed,
      zlib_decompressed,
      zstd_decompressed,
      lzma_decompressed;

flt snappy_compress_time,
       lz4_compress_time,
      zlib_compress_time,
      zstd_compress_time,
      lzma_compress_time,

    snappy_decompress_time,
       lz4_decompress_time,
      zlib_decompress_time,
      zstd_decompress_time,
      lzma_decompress_time;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   // read the source file, we'll use "engine.pak" because it's big enough
   src.read(EE_ENGINE_PATH);

   // compress the source into files in memory, using different compression algorithms, and measure the time
   snappy_compress_time=Time.curTime(); src.pos(0); snappy_compressed.writeMem(); Compress(src, snappy_compressed, COMPRESS_SNAPPY); snappy_compress_time=Time.curTime()-snappy_compress_time;
      lz4_compress_time=Time.curTime(); src.pos(0);    lz4_compressed.writeMem(); Compress(src,    lz4_compressed, COMPRESS_LZ4   );    lz4_compress_time=Time.curTime()-   lz4_compress_time;
     zlib_compress_time=Time.curTime(); src.pos(0);   zlib_compressed.writeMem(); Compress(src,   zlib_compressed, COMPRESS_ZLIB  );   zlib_compress_time=Time.curTime()-  zlib_compress_time;
     zstd_compress_time=Time.curTime(); src.pos(0);   zstd_compressed.writeMem(); Compress(src,   zstd_compressed, COMPRESS_ZSTD  );   zstd_compress_time=Time.curTime()-  zstd_compress_time;
     lzma_compress_time=Time.curTime(); src.pos(0);   lzma_compressed.writeMem(); Compress(src,   lzma_compressed, COMPRESS_LZMA  );   lzma_compress_time=Time.curTime()-  lzma_compress_time;

   // decompress the data, and measure the time
   snappy_decompress_time=Time.curTime(); snappy_compressed.pos(0); snappy_decompressed.writeMem(); Decompress(snappy_compressed, snappy_decompressed); snappy_decompress_time=Time.curTime()-snappy_decompress_time;
      lz4_decompress_time=Time.curTime();    lz4_compressed.pos(0);    lz4_decompressed.writeMem(); Decompress(   lz4_compressed,    lz4_decompressed);    lz4_decompress_time=Time.curTime()-   lz4_decompress_time;
     zlib_decompress_time=Time.curTime();   zlib_compressed.pos(0);   zlib_decompressed.writeMem(); Decompress(  zlib_compressed,   zlib_decompressed);   zlib_decompress_time=Time.curTime()-  zlib_decompress_time;
     zstd_decompress_time=Time.curTime();   zstd_compressed.pos(0);   zstd_decompressed.writeMem(); Decompress(  zstd_compressed,   zstd_decompressed);   zstd_decompress_time=Time.curTime()-  zstd_decompress_time;
     lzma_decompress_time=Time.curTime();   lzma_compressed.pos(0);   lzma_decompressed.writeMem(); Decompress(  lzma_compressed,   lzma_decompressed);   lzma_decompress_time=Time.curTime()-  lzma_decompress_time;

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
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);

   TextStyleParams ts; ts.size=0.07;
   D.text(ts, 0,  0.6, S+"Source Size: "+(src.size()>>10)+"KB");
   D.text(ts, 0,  0.4, S+"SNAPPY - Compressed Size: "+(snappy_compressed.size()>>10)+"KB, Compression / Decompression Time: "+snappy_compress_time+" / "+snappy_decompress_time);
   D.text(ts, 0,  0.2, S+"LZ4 - Compressed Size: "   +(   lz4_compressed.size()>>10)+"KB, Compression / Decompression Time: "+   lz4_compress_time+" / "+   lz4_decompress_time);
   D.text(ts, 0,  0.0, S+"ZLIB - Compressed Size: "  +(  zlib_compressed.size()>>10)+"KB, Compression / Decompression Time: "+  zlib_compress_time+" / "+  zlib_decompress_time);
   D.text(ts, 0, -0.2, S+"ZSTD - Compressed Size: "  +(  zstd_compressed.size()>>10)+"KB, Compression / Decompression Time: "+  zstd_compress_time+" / "+  zstd_decompress_time);
   D.text(ts, 0, -0.4, S+"LZMA - Compressed Size: "  +(  lzma_compressed.size()>>10)+"KB, Compression / Decompression Time: "+  lzma_compress_time+" / "+  lzma_decompress_time);
}
/******************************************************************************/
