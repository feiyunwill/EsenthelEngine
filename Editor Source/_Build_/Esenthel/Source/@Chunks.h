/******************************************************************************/
class Chunks
{
   Memc<Chunk> chunks;

   static int Compare(C Chunk &a, C Chunk &b);

   Chunk* findChunk(C Str &name);                     
   Chunks& delChunk(C Str &name);                     
   Chunks& setChunk(C Str &name, uint ver, File&file);

   void del();

   // io
   bool load(File &f);
   bool save(File &f); // warning: this sorts 'chunks' and changes memory addresses for each element
   bool load(C Str &name, ReadWriteSync &rws);
   bool save(C Str &name, ReadWriteSync &rws); // warning: this sorts 'chunks' and changes memory addresses for each element
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
