/******************************************************************************

   Use following functions to handle data compression.

/******************************************************************************/
enum COMPRESS_TYPE : Byte // Compression Library Type
{
   COMPRESS_NONE  , // don't use               compression, best    performance (26x), biggest  size (100%), data is stored in original format
   COMPRESS_ZLIB  , // use Zlib                compression, medium  performance ( 5x), small    size (~64%)
   COMPRESS_LZMA  , // use LZMA                compression, slowest performance ( 1x), smallest size (~55%), it can have high memory usage for large data and high compression level, please see 'CompressionMemUsage' for more info
   COMPRESS_SNAPPY, // use Google Snappy       compression, good    performance (16x), medium   size (~72%), the library was designed to achieve fastest compression/decompression speed at the cost of lower compression when compared to Zlib
   COMPRESS_RLE   , // use Run-Length Encoding compression, good    performance (23x), big      size (   ?), compression can be good only for memory in which can be found continuous chunks of bytes of the same value
   COMPRESS_LZ4   , // use LZ4                 compression, good    performance (16x), medium   size (~70%), the library was designed to achieve fastest compression/decompression speed at the cost of lower compression when compared to Zlib, compression is slower than Snappy, however decompression is faster and the file sizes are smaller
   COMPRESS_LZHAM , // use LZHAM               compression, slow    performance ( 2x), small    size (~57%), compression is slower than LZMA and it achieves slightly worse compression rates than LZMA, however decompression speeds are 1.5-8x higher
   COMPRESS_ZSTD  , // use Zstd                compression, medium  performance ( 8x), small    size (~63%), compression is slower than ZLIB and it achieves similar        compression rates to   ZLIB, however decompression speeds are     2x higher
#if EE_PRIVATE
   COMPRESS_BROTLI, // use Google Brotli       compression,         performance      ,          size
   COMPRESS_LIZARD, // use LIZARD              compression,         performance      ,          size
#endif
   COMPRESS_NUM   , // number of compression types
};
/******************************************************************************/
struct DataCallback // you can override this class and pass its object into 'Compress' or 'Decompress' functions, it will get called for every uncompressed data chunk, you can use it for example to calculate uncompressed data hash
{
   virtual void reset(                   )=NULL; // called when data stream needs to be reset
   virtual void data (CPtr data, Int size)=NULL; // called when processed 'data' of 'size'
};
struct xxHash32Calc : DataCallback
{
   xxHash32 hash;

   virtual void reset(                   )override {hash.reset();}
   virtual void data (CPtr data, Int size)override {hash.update(data, size);}
};
struct xxHash64Calc : DataCallback
{
   xxHash64 hash;

   virtual void reset(                   )override {hash.reset();}
   virtual void data (CPtr data, Int size)override {hash.update(data, size);}
};
/******************************************************************************/
Bool Compressable(C Str &file_extension); // if 'file_extension' is compressable (compressing most likely would reduce the file size, this returns false for file types already compressed like "jpg", "mp3", "avi", "rar", .. and true for all other file types)

CChar8* CompressionName    (COMPRESS_TYPE type); // get compression type name in text format
VecI2   CompressionLevels  (COMPRESS_TYPE type); // get range of compression levels for specified type, 'VecI2.x' will contain the minimum level, 'VecI2.y' will contain the maximum level, if both values are the same, then it means that this type does not support different compression levels
UInt    CompressionMemUsage(COMPRESS_TYPE type, Int compression_level=9, Long uncompressed_size=-1); // get memory usage used for   compression, 'uncompressed_size'=size of the uncompressed data (use -1 if unknown)
UInt  DecompressionMemUsage(COMPRESS_TYPE type, Int compression_level=9, Long uncompressed_size=-1); // get memory usage used for decompression, 'uncompressed_size'=size of the uncompressed data (use -1 if unknown)

// perform file->file (de)compression
Bool   Compress(File &src, File &dest, COMPRESS_TYPE type, Int compression_level=9, Bool multi_threaded=true, DataCallback *callback=null); //   compress data from 'src' file into 'dest' file, 'src' should be already opened for reading, 'dest' should be already opened for writing, 'compression_level'=0..CompressionLevels(type) (0=fastest/worst, ..=slowest/best), compression occurs from the current position of 'src' to the end of the file, 'callback'=optional callback which will be called for every uncompressed data chunk (you can use it for example to calculate uncompressed data hash), false on fail
Bool Decompress(File &src, File &dest, Bool memory=false                                                    , DataCallback *callback=null); // decompress data from 'src' file into 'dest' file, 'src' should be already opened for reading, 'dest' should be already opened for writing if 'memory' is set to false, if 'memory' is set to true then decompression will be faster, however 'dest' will be first reinitialized with 'writeMemFixed' before decompressing, which means that decompression result will not be stored into original 'dest' target, but instead into a dynamically allocated memory, 'callback'=optional callback which will be called for every uncompressed data chunk (you can use it for example to calculate uncompressed data hash), false on fail

// perform file->file raw (de)compression without storing any header
Bool   CompressRaw(File &src, File &dest, COMPRESS_TYPE type, Int compression_level=9, Bool multi_threaded=true                , DataCallback *callback=null); //   compress data from 'src' file into 'dest' file, 'src' should be already opened for reading, 'dest' should be already opened for writing, 'compression_level'=0..CompressionLevels(type) (0=fastest/worst, ..=slowest/best), compression occurs from the current position of 'src' to the end of the file, 'callback'=optional callback which will be called for every uncompressed data chunk (you can use it for example to calculate uncompressed data hash), false on fail
Bool DecompressRaw(File &src, File &dest, COMPRESS_TYPE type, ULong compressed_size, ULong decompressed_size, Bool memory=false, DataCallback *callback=null); // decompress data from 'src' file into 'dest' file, 'src' should be already opened for reading, 'dest' should be already opened for writing if 'memory' is set to false, if 'memory' is set to true then decompression will be faster, however 'dest' will be first reinitialized with 'writeMemFixed' before decompressing, which means that decompression result will not be stored into original 'dest' target, but instead into a dynamically allocated memory, 'compressed_size'=size of compressed data in 'src', 'decompressed_size'=precise size of decompressed data, 'callback'=optional callback which will be called for every uncompressed data chunk (you can use it for example to calculate uncompressed data hash), false on fail

#if EE_PRIVATE

#define MIN_COMPRESSABLE_SIZE 3 // 3 is the minimum size that can actually get compressed (by COMPRESS_RLE)

Bool DecompressHeader(File &src, COMPRESS_TYPE &type, ULong &compressed_size, ULong &decompressed_size);

const Int MaxCmpUIntVSize =5,
          MaxCmpULongVSize=9;
      Int     CmpUIntVSize(UInt u); // get number of bytes needed for storing 'u' using 'cmpUIntV' algorithm
#endif
/******************************************************************************/
