/******************************************************************************

   Chunks (ChunkWriter and ChunkReader) are special classess allowing to 
      store multiple chunks of custom data in a single file.

   The main advantage of using chunks, is that while reading a single file with multiple chunks inside,
      you can skip reading chunks which you're not interested in, without knowing their size in the file.

   Chunks are stored and accessed sequentially.

   Each chunk contains a name and a version number.
   
   This is a sample file with chunks:
   
      File : "data.chunk"
      {
         ChunkListBegin
         {
            Chunk(name="A Chunk", version=0)
            {
               custom data
            {
            Chunk(name="Some other chunk", version=2)
            {
               custom data
            }
            Chunk(name="Repeated data", version=0)
            {
               custom data
            }
            Chunk(name="Repeated data", version=0)
            {
               custom data
            }
         }
         ChunkListEnd
      }

   In this tutorial we'll present writing multiple chunks to a file (in memory),
      and then accessing them sequentially, but skipping unwanted chunks.

/******************************************************************************/
Memc<Str> strings; // container of strings used for testing
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
void WriteChunks(File &f)
{
   ChunkWriter cw; // create a chunk writer

   cw.beginChunkList(f); // begin chunk list
   {
      if(File *chunk=cw.beginChunk("A Chunk", 0)) // write "A Chunk" chunk with 'version'=0
      {
         // now when the chunk has been created we can write any custom data inside it
         // lets put some random bytes
         chunk.putByte(123);
         chunk.putByte( 16);
         chunk.putByte( 32);
      }
      
      if(File *chunk=cw.beginChunk("Some other chunk", 2))
      {
         // now we can write custom data to the other chunk
         // lets put a float this time
         chunk.putFlt(123.456);
      }
      
      if(File *chunk=cw.beginChunk("Repeated Data", 0))
      {
         // now lets write a data which we'll actually use later
         // lets put a string
         chunk.putStr("First String");
      }
      
      if(File *chunk=cw.beginChunk("Repeated Data", 0))
      {
         // write another chunk with the same name
         // lets put a different string this time
         chunk.putStr("Second String");
      }
   }
   cw.endChunkList(); // now when all chunks have been written, we should write the ending marker
}
/******************************************************************************/
void ReadChunks(File &f)
{
   ChunkReader cr; // create a chunk reader
   if(cr.read(f)) // start reading chunks
   {
      for(; File *chunk=cr(); ) // continuously process while chunks are available
      {
         // now we can process the chunk data or not
         // so let's say we only want to process chunks with the name "Repeated Data" 

         if(cr.name()=="Repeated Data") // if the name of the chunk is matched
         {
            // now we should check if we support the version of the written chunk
            switch(cr.ver())
            {
               case 0: // we've written the data for "Repeated Data" chunks using version=0
               {
                  strings.add(chunk.getStr()); // read the string which we've written earlier and store it in a temporary container to draw it on the screen later
               }break;
            }
         }/*else
         if(cr.name()==..) // here you can check for other chunks if you'd like to process them
         {
            
         }*/
      }
   }
   // here all chunks have been processed
}
/******************************************************************************/
Bool Init()
{
   File f; f.writeMem(); // create a file and start writing to memory

   WriteChunks(f);

   // now the 'f' file contains multiple chunks of data
   // so lets see how it works, and lets try to access the data using

   f.pos(0); // first let's reset the file position to the start, because after writing the chunks the file position was at the end

   ReadChunks(f);

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
Bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);

   // draw all strings from 'Repeated Data' chunks on the screen
   FREPA(strings)D.text(0, 0.9-i*0.1, strings[i]);
}
/******************************************************************************/
