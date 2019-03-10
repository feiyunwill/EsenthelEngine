/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
#define CC4_CHUNK CC4('C','H','N','K')
/******************************************************************************/
// WRITER
/******************************************************************************/
ChunkWriter::~ChunkWriter()
{
   endChunk(); // end current chunk if any, but not the list, in case we want to append to it later
  _f=null;
  _chunk_size_pos=0;
}
ChunkWriter::ChunkWriter()
{
  _f=null;
  _chunk_size_pos=0;
}
ChunkWriter::ChunkWriter(const_mem_addr File &f) : ChunkWriter() {beginChunkList(f);}
/******************************************************************************/
void ChunkWriter::appendChunkList(File &f)
{
   endChunk(); // end current chunk if any, but not the list, because we're appending it now (if it exists)
  _chunk_size_pos=0;
  _f=&f;
   if(!f.pos())f.putUInt(CC4_CHUNK); // automatically add marker only when the file is at start
}
/******************************************************************************/
void ChunkWriter::beginChunkList(File &f)
{
   endChunkList(); // end current chunk list if any
  _chunk_size_pos=0;
  _f=&f;
   f.putUInt(CC4_CHUNK); // add chunk marker
}
/******************************************************************************/
File* ChunkWriter::beginChunk(C Str &name, UInt version)
{
   if(_f)
   {
      endChunk(); // finish current chunk if any
                                _f->cmpUIntV(3      ); // internal chunk container version
                                _f->putStr  (name   ); // chunk name
                                _f->cmpUIntV(version); // chunk version
     _chunk_size_pos=_f->pos(); _f->putUInt (0      ); // remember position of chunk size just before writing it, to update it later, keep as fixed UInt size because we don't know it yet
   }
   return _f;
}
/******************************************************************************/
void ChunkWriter::endChunk()
{
   if(_chunk_size_pos) // we can end a chunk only if we started one
   {
      // update size of last written chunk
      ULong cur_pos=_f->pos(); // remember current position
     _f->pos    (_chunk_size_pos);
     _f->putUInt(cur_pos-_chunk_size_pos-SIZE(UInt));
     _f->pos    (cur_pos); // restore last position
     _chunk_size_pos=0; // clear chunk size pos as it's now ended
   }
}
/******************************************************************************/
void ChunkWriter::endChunkList()
{
   if(_f)
   {
      endChunk(); // end current chunk if any
     _f->cmpUIntV(0); // end of list, normally this is the place for chunk container version, setting 0 means that there are no more chunks, this is also compatible with 0 always being returned at the end of file
     _f=null;
   }
}
/******************************************************************************/
// READER
/******************************************************************************/
void ChunkReader::zero()
{
  _ver=0; _next_chunk_pos=0; _full_size=0; _offset_delta=0; _f=null;
}
ChunkReader::~ChunkReader() {restore();} // restore in case not all chunks were processed
ChunkReader:: ChunkReader() {zero   ();}
ChunkReader:: ChunkReader(const_mem_addr File &f) : ChunkReader() {read(f);}

void ChunkReader::restore()
{
   if(_f)_f->unlimit(_full_size, _offset_delta); // restore original file values
}
/******************************************************************************/
Bool ChunkReader::read(File &f)
{
   restore();
   zero();
  _name.clear();
   if(f.getUInt()==CC4_CHUNK)
   {
      T._f             =&f;
      T._full_size     = f.size();
      T._next_chunk_pos= f.pos (); return true;
   }
   return false;
}
/******************************************************************************/
File* ChunkReader::operator()()
{
   if(_f)
   {
      restore(); // restore so that we can read the chunk container

      // go to next chunk
     _f->pos(_next_chunk_pos);
      if(!_f->end()) // don't read more at the end of file because that would trigger an error (f.ok=false), this is because chunks are normally processed in this way: "for(ChunkReader cr(f); File *f=cr(); )", keep reading as long as they exist
         switch(_f->decUIntV()) // read internal chunk container version
      {
       //default: _f->_ok=false; break; // unsupported version

         case 3:
         {
            // load chunk info
           _f->getStr  (_name);
           _f->decUIntV(_ver );
            UInt chunk_size=_f->getUInt();
            if(_f->ok())
            {
              _next_chunk_pos=_f->pos()+chunk_size; // current position + chunk size
              _f->limit(_full_size, _offset_delta, chunk_size);
               return _f;
            }
         }break;

         case 2:
         {
            // load chunk info
           _f->_getStr2 (_name);
           _f-> decUIntV(_ver );
            UInt chunk_size=_f->getUInt();
            if(_f->ok())
            {
              _next_chunk_pos=_f->pos()+chunk_size; // current position + chunk size
              _f->limit(_full_size, _offset_delta, chunk_size);
               return _f;
            }
         }break;

         case 1:
         {
            // load chunk info
              _f->_getStr(_name);
            (*_f)>>_ver;
            UInt chunk_size=_f->getUInt();
            if(_f->ok())
            {
              _next_chunk_pos=_f->pos()+chunk_size; // current position + chunk size
              _f->limit(_full_size, _offset_delta, chunk_size);
               return _f;
            }
         }break;

         case 0: break; // end of list
      }
   }
  _name.clear();
  _ver =0;
   return null;
}
/******************************************************************************/
}
/******************************************************************************/
