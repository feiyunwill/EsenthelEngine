/******************************************************************************

   Use 'ChunkWriter' and 'ChunkReader' classes to write and read binary data using chunks.

   Each chunk has a:
      -custom name       (Str )
      -custom version ID (UInt)
      -size of the chunk (UInt)
      -custom chunk data (array of bytes)

   Chunks are useful if you want to save many different types of data,
      however when reading the chunks later, skip those which you're not interested in,
      making the 'ChunkReader' class automatically proceed to the next chunks,
      without the need of manual manipulating the file positions.

/******************************************************************************/
struct ChunkWriter // Writes a custom amount of chunks into a single file
{
   void appendChunkList(const_mem_addr File &f   ); // continue writing to existing chunk list to 'f' file, this method writes "chunk list marker" only if the file is empty, 'f' shouldn't be modified (or closed) during chunk data saving, because 'ChunkWriter' relies on 'f'
   void  beginChunkList(const_mem_addr File &f   ); // begin    writing a  new      chunk list to 'f' file, this method writes "chunk list marker" every time               , 'f' shouldn't be modified (or closed) during chunk data saving, because 'ChunkWriter' relies on 'f', automatically calls 'endChunkList' at the start if needed
   File* beginChunk    (C Str &name, UInt version); // begin    writing a 'name' chunk in current file with 'version', this method returns a pointer to a file where you should write your custom data, null on fail, automatically calls 'endChunk' at the start if needed
   void    endChunk    (                         ); // end      writing a        chunk in current file
   void    endChunkList(                         ); // end      writing chunk list, this method writes an ending marker at the end of all chunks, automatically calls 'endChunk' at the start if needed

           ~ChunkWriter(                      ); // automatically calls 'endChunk' if needed, however does not call 'endChunkList' so the file can have new chunks added later using 'appendChunkList'
            ChunkWriter(                      );
   explicit ChunkWriter(const_mem_addr File &f); // automatically calls 'beginChunkList'

private:
   ULong _chunk_size_pos;
   File *_f;
   NO_COPY_CONSTRUCTOR(ChunkWriter);
};
/******************************************************************************/
struct ChunkReader // Processes chunks from a single file
{
   // manage
   Bool read(const_mem_addr File &f); // start reading chunks from 'f' file, false on fail, 'f' shouldn't be modified (or closed) until 'ChunkReader' finishes processing chunks because 'ChunkReader' relies on 'f'

   // get
   File* operator()();                 // get next chunk for processing, null on fail
 C Str & name      ()C {return _name;} // get name    of currently processed chunk, this will contain valid data after 'operator()' was called
   UInt  ver       ()C {return _ver ;} // get version of currently processed chunk, this will contain valid data after 'operator()' was called

#if EE_PRIVATE
   void zero   ();
   void restore();
#endif

           ~ChunkReader(                      );
            ChunkReader(                      );
   explicit ChunkReader(const_mem_addr File &f); // automatically calls 'read'

private:
   UInt  _ver;
   ULong _next_chunk_pos, _full_size, _offset_delta;
   Str   _name;
   File *_f;
   NO_COPY_CONSTRUCTOR(ChunkReader);
};
/******************************************************************************/
