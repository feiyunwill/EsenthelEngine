//-------------------------------------------------------------------------------------
// DirectXMeshOptimize.cpp
//  
// DirectX Mesh Geometry Library - Mesh optimization
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkID=324981
//-------------------------------------------------------------------------------------

namespace DirectX
{

#define UNUSED32 (0xffffffff)
#undef  UINT32_MAX
#define UINT32_MAX UINT_MAX
#pragma warning(disable:4267)

enum OPTFACES
{
   OPTFACES_V_DEFAULT      = 12,
   OPTFACES_R_DEFAULT      = 7,
      // Default vertex cache size and restart threshold which is considered 'device independent'

   OPTFACES_V_STRIPORDER   = 0,
      // Indicates no vertex cache optimization, only reordering into strips
};

template<class index_t>
static inline uint32_t find_edge( _In_reads_(3) const index_t* indices, index_t search )
{
   assert( indices != 0 );

   uint32_t edge = 0;

   for( ; edge < 3; ++edge )
   {
      if ( indices[ edge ] == search )
            break;
   }

   return edge;
}

//=====================================================================================
// Attribute Utilities
//=====================================================================================
_Use_decl_annotations_
static void ComputeSubsets( std__vector< std__pair<size_t,size_t> > &subsets, const uint32_t* attributes, size_t nFaces )
{
   subsets.clear();

    if ( !nFaces )return;
    if ( !attributes )
    {
        subsets.emplace_back( std__pair<size_t,size_t>( 0, nFaces ) );
        return;
    }

    uint32_t lastAttr = attributes[ 0 ];
    size_t offset = 0;
    size_t count = 1;

    for( size_t j = 1; j < nFaces; ++j )
    {
        if ( attributes[ j ] != lastAttr )
        {
            subsets.emplace_back( std__pair<size_t,size_t>( offset, count ) );
            lastAttr = attributes[ j ];
            offset = j;
            count = 1;
        }
        else
        {
            count += 1;
        }
    }

    if ( count > 0 )subsets.emplace_back( std__pair<size_t,size_t>( offset, count ) );
}

//-------------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable:6101)

template<class index_t>
static Bool ReorderFaces( _In_reads_(nFaces*3) const index_t* ibin, _In_ size_t nFaces,
                        _In_reads_opt_(nFaces*3) const uint32_t* adjin, 
                        _In_reads_(nFaces) const uint32_t* faceRemap,
                        _Out_writes_(nFaces*3) index_t* ibout,
                        _Out_writes_opt_(nFaces*3) uint32_t* adjout )
{
    assert( ibin != 0 && faceRemap != 0 && ibout != 0 && ibin != ibout );
    _Analysis_assume_( ibin != 0 && faceRemap != 0 && ibout != 0 && ibin != ibout );

    assert( ( !adjin && !adjout ) || ( (adjin && adjout) && adjin != adjout ) );
    _Analysis_assume_( ( !adjin && !adjout ) || ( (adjin && adjout) && adjin != adjout ) );
    
    for( size_t j = 0; j < nFaces; ++j )
    {
        uint32_t src = faceRemap[ j ];

        if ( src == UNUSED32 )
            continue;

        if ( src < nFaces )
        {
            ibout[ j*3 ] = ibin[ src*3 ]; 
            ibout[ j*3 + 1 ] = ibin[ src*3 + 1 ]; 
            ibout[ j*3 + 2 ] = ibin[ src*3 + 2 ];

            if ( adjin && adjout )
            {
                adjout[ j*3 ] = adjin[ src*3 ]; 
                adjout[ j*3 + 1 ] = adjin[ src*3 + 1 ]; 
                adjout[ j*3 + 2 ] = adjin[ src*3 + 2 ];
            }
        }
        else
            return false;
    }
    
    return true;
}

#pragma warning(pop)


//-------------------------------------------------------------------------------------
template<class index_t>
static Bool SwapFaces( _Inout_updates_all_(nFaces*3) index_t* ib, _In_ size_t nFaces,
                    _Inout_updates_all_opt_(nFaces*3) uint32_t* adj,
                    _In_reads_(nFaces) const uint32_t* faceRemap)
{
    assert( ib != 0 && faceRemap != 0 );
    _Analysis_assume_( ib != 0 && faceRemap != 0 );

    std__unique_ptr<uint8_t> temp( ( sizeof(bool) + sizeof(uint32_t) ) * nFaces );
    if ( !temp )
        return false;

    uint32_t *faceRemapInverse = reinterpret_cast<uint32_t*>( temp.get() );

    for( uint32_t j = 0; j < nFaces; ++j )
    {
        faceRemapInverse[ faceRemap[ j ] ] = j;
    }

    bool *moved = reinterpret_cast<bool*>( temp.get() + sizeof(uint32_t) * nFaces );

    memset( moved, 0, sizeof(bool) * nFaces );

    for( size_t j = 0; j < nFaces; ++j )
    {
        if ( moved[ j ] )
            continue;

        uint32_t dest = faceRemapInverse[ j ];

        if ( dest == UNUSED32 )
            continue;

        if ( dest >= nFaces )
            return false;

        while( dest != j )
        {
            // Swap face
            index_t i0 = ib[ dest*3 ];
            index_t i1 = ib[ dest*3 + 1 ];
            index_t i2 = ib[ dest*3 + 2 ];

            ib[ dest*3 ] = ib[ j*3 ];
            ib[ dest*3 + 1 ] = ib[ j*3 + 1 ];
            ib[ dest*3 + 2 ] = ib[ j*3 + 2 ];

            ib[ j*3 ] = i0;
            ib[ j*3 + 1 ] = i1;
            ib[ j*3 + 2 ] = i2;

            if ( adj )
            {
                uint32_t a0 = adj[ dest*3 ];
                uint32_t a1 = adj[ dest*3 + 1 ];
                uint32_t a2 = adj[ dest*3 + 2 ];

                adj[ dest*3 ] = adj[ j*3 ];
                adj[ dest*3 + 1 ] = adj[ j*3 + 1 ];
                adj[ dest*3 + 2 ] = adj[ j*3 + 2 ];

                adj[ j*3 ] = a0;
                adj[ j*3 + 1 ] = a1;
                adj[ j*3 + 2 ] = a2;
            }

            moved[ dest ] = true;

            dest = faceRemapInverse[ dest ];

            if ( dest == UNUSED32 || moved[dest] )
                break;

            if ( dest >= nFaces )
                return false;
        }
    }

    return true;
}


//-------------------------------------------------------------------------------------
static Bool SwapVertices( _Inout_updates_bytes_all_(nVerts*stride) void* vb, size_t stride, size_t nVerts,
                      _Inout_updates_all_opt_(nVerts) uint32_t* pointRep, _In_reads_(nVerts) const uint32_t* vertexRemap )
{
    if ( !vb || !stride || !nVerts || !vertexRemap )
        return false;

    std__unique_ptr<uint8_t> temp( nVerts + stride );
    if ( !temp )
        return false;

    uint8_t *moved = temp.get();
    memset( moved, 0, nVerts );

    uint8_t *vbtemp = temp.get() + nVerts;

    uint8_t *ptr = reinterpret_cast<uint8_t*>( vb );

    for( size_t j = 0; j < nVerts; ++j )
    {
        if ( moved[j] )
            continue;

        uint32_t dest = vertexRemap[ j ];

        if ( dest == UNUSED32 )
            continue;

        if ( dest >= nVerts )
            return false;

        bool next = false;

        while( dest != j )
        {
            // Swap vertex
            #pragma prefast(push)
            #pragma prefast(disable : 26019, "PREfast noise: Esp:1307")
            memcpy( vbtemp, ptr + dest*stride, stride );
            memcpy( ptr + dest*stride, ptr + j*stride, stride );
            memcpy( ptr + j*stride, vbtemp, stride );
            #pragma prefast(pop)

            if ( pointRep )
            {
                Swap( pointRep[ dest ], pointRep[ j ] );
                // Remap
                uint32_t pr = pointRep[ dest ];
                if ( pr < nVerts )
                {
                    pointRep[ dest ] = vertexRemap[ pr ];
                }
            }

            moved[ dest ] = true;

            dest = vertexRemap[ dest ];

            if ( dest == UNUSED32 || moved[dest] )
            {
                next = true;
                break;
            }

            if ( dest >= nVerts )
                return false;
        }

        if ( next )
            continue;

        if ( pointRep )
        {
            // Remap
            uint32_t pr = pointRep[ j ];
            if ( pr < nVerts )
            {
                pointRep[ j ] = vertexRemap[ pr ];
            }
        }
    }

    return true;
}


//-------------------------------------------------------------------------------------
template<class index_t>
static Bool _FinalizeIB( _In_reads_(nFaces*3) const index_t* ibin, size_t nFaces,
                     _In_reads_(nVerts) const uint32_t* vertexRemap, size_t nVerts,
                     _Out_writes_(nFaces*3) index_t* ibout )
{
    if ( !ibin || !nFaces || !vertexRemap || !nVerts || !ibout )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    if ( nVerts >= index_t(-1) )
        return false;

    for( size_t j = 0; j < ( nFaces * 3 ); ++j )
    {
        index_t i = ibin[ j ];
        if ( i == index_t(-1) )
        {
            ibout[ j ] = index_t(-1);
            continue;
        }

        if ( i >= nVerts )
            return false;

        uint32_t dest = vertexRemap[ i ];
        if ( dest == UNUSED32 )
        {
            ibout[ j ] = i;
            continue;
        }

        if ( dest < nVerts )
        {
            ibout[ j ] = index_t( dest );
        }
        else
            return false;
    }

    return true;
}


//-------------------------------------------------------------------------------------
template<class index_t>
static Bool _FinalizeIB( _In_reads_(nFaces*3) index_t* ib, size_t nFaces, _In_reads_(nVerts) const uint32_t* vertexRemap, size_t nVerts )
{
    if ( !ib || !nFaces || !vertexRemap || !nVerts )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    if ( nVerts >= index_t(-1) )
        return false;

    for( size_t j = 0; j < ( nFaces * 3 ); ++j )
    {
        index_t i = ib[ j ];
        if( i == index_t(-1) )
            continue;

        if ( i >= nVerts )
            return false;

        uint32_t dest = vertexRemap[ i ];
        if ( dest == UNUSED32 )
            continue;

        if ( dest < nVerts )
        {
            ib[ j ] = index_t( dest );
        }
        else
            return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------
// Applies a face remap reordering to an index buffer
//-------------------------------------------------------------------------------------
_Use_decl_annotations_
static Bool ReorderIB( const uint16_t* ibin, size_t nFaces, const uint32_t* faceRemap, uint16_t* ibout )
{
    if ( !ibin || !nFaces || !faceRemap || !ibout )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    if ( ibin == ibout )
        return false;

    return ReorderFaces<uint16_t>( ibin, nFaces, nullptr, faceRemap, ibout, nullptr );
}

_Use_decl_annotations_
static Bool ReorderIB( uint16_t* ib, size_t nFaces, const uint32_t* faceRemap )
{
    if ( !ib || !nFaces || !faceRemap )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    return SwapFaces<uint16_t>( ib, nFaces, nullptr, faceRemap );
}


//-------------------------------------------------------------------------------------
_Use_decl_annotations_
static Bool ReorderIB( const uint32_t* ibin, size_t nFaces, const uint32_t* faceRemap, uint32_t* ibout )
{
    if ( !ibin || !nFaces || !faceRemap || !ibout )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    if ( ibin == ibout )
        return false;

    return ReorderFaces<uint32_t>( ibin, nFaces, nullptr, faceRemap, ibout, nullptr );
}

_Use_decl_annotations_
static Bool ReorderIB( uint32_t* ib, size_t nFaces, const uint32_t* faceRemap )
{
    if ( !ib || !nFaces || !faceRemap )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    return SwapFaces<uint32_t>( ib, nFaces, nullptr, faceRemap );
}


//-------------------------------------------------------------------------------------
// Applies a face remap reordering to an index buffer and adjacency
//-------------------------------------------------------------------------------------
_Use_decl_annotations_
static Bool ReorderIBAndAdjacency( const uint16_t* ibin, size_t nFaces, const uint32_t* adjin, const uint32_t* faceRemap,
                               uint16_t* ibout, uint32_t* adjout )
{
    if ( !ibin || !nFaces || !adjin || !faceRemap || !ibout || !adjout )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    if ( ( ibin == ibout ) || ( adjin == adjout ) )
        return false;

    return ReorderFaces<uint16_t>( ibin, nFaces, adjin, faceRemap, ibout, adjout );
}

_Use_decl_annotations_
static Bool ReorderIBAndAdjacency( uint16_t* ib, size_t nFaces, uint32_t* adj, const uint32_t* faceRemap )
{
    if ( !ib || !nFaces || !adj || !faceRemap )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    return SwapFaces<uint16_t>( ib, nFaces, adj, faceRemap );
}


//-------------------------------------------------------------------------------------
_Use_decl_annotations_
static Bool ReorderIBAndAdjacency( const uint32_t* ibin, size_t nFaces, const uint32_t* adjin, const uint32_t* faceRemap,
                               uint32_t* ibout, uint32_t* adjout )
{
    if ( !ibin || !nFaces || !adjin || !faceRemap || !ibout || !adjout )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    if ( ( ibin == ibout ) || ( adjin == adjout ) )
        return false;

    return ReorderFaces<uint32_t>( ibin, nFaces, adjin, faceRemap, ibout, adjout );
}

_Use_decl_annotations_
static Bool ReorderIBAndAdjacency( uint32_t* ib, size_t nFaces, uint32_t* adj, const uint32_t* faceRemap )
{
    if ( !ib || !nFaces || !adj || !faceRemap )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    return SwapFaces<uint32_t>( ib, nFaces, adj, faceRemap );
}


//-------------------------------------------------------------------------------------
// Applies a vertex remap, filling out a new index buffer
//-------------------------------------------------------------------------------------
_Use_decl_annotations_
static Bool FinalizeIB( const uint16_t* ibin, size_t nFaces, const uint32_t* vertexRemap, size_t nVerts, uint16_t* ibout )
{
    return _FinalizeIB<uint16_t>( ibin, nFaces, vertexRemap, nVerts, ibout );
}

_Use_decl_annotations_
static Bool FinalizeIB( uint16_t* ib, size_t nFaces, const uint32_t* vertexRemap, size_t nVerts )
{
    return _FinalizeIB<uint16_t>( ib, nFaces, vertexRemap, nVerts );
}


//-------------------------------------------------------------------------------------
_Use_decl_annotations_
static Bool FinalizeIB( const uint32_t* ibin, size_t nFaces, const uint32_t* vertexRemap, size_t nVerts, uint32_t* ibout )
{
    return _FinalizeIB<uint32_t>( ibin, nFaces, vertexRemap, nVerts, ibout );
}

_Use_decl_annotations_
static Bool FinalizeIB( uint32_t* ib, size_t nFaces, const uint32_t* vertexRemap, size_t nVerts )
{
    return _FinalizeIB<uint32_t>( ib, nFaces, vertexRemap, nVerts );
}


//-------------------------------------------------------------------------------------
// Applies a vertex remap and/or a vertex duplication set to a vertex buffer
//-------------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning( disable : 6101 )

_Use_decl_annotations_
static Bool FinalizeVB( const void* vbin, size_t stride, size_t nVerts,
                    const uint32_t* dupVerts, size_t nDupVerts,
                    const uint32_t* vertexRemap, void* vbout )
{
    if ( !vbin || !stride || !nVerts || !vbout )
        return false;

    if ( !dupVerts && !vertexRemap )
        return false;

    if ( dupVerts && !nDupVerts )
        return false;

    if ( !dupVerts && nDupVerts > 0 )
        return false;

    if ( nVerts >= UINT32_MAX )
        return false;

    if ( (uint64_t(nVerts) + uint64_t(nDupVerts) ) >= UINT32_MAX )
        return false;

    if ( vbin == vbout )
        return false;

    size_t newVerts = nVerts + nDupVerts;

    const uint8_t *sptr = reinterpret_cast<const uint8_t*>( vbin );
          uint8_t *dptr = reinterpret_cast<uint8_t*>( vbout );

#if DEBUG
    memset( vbout, 0, newVerts * stride );
#endif

    for( size_t j = 0; j < nVerts; ++j )
    {
        uint32_t dest = ( vertexRemap ) ? vertexRemap[ j ] : uint32_t(j);

        if ( dest == UNUSED32 )
        {
            // remap entry is unused
        }
        else if ( dest < newVerts )
        {
            memcpy( dptr + dest * stride, sptr, stride ); 
        }
        else
            return false;

        sptr += stride;
    }

    if ( dupVerts )
    {
        for( size_t j = 0; j < nDupVerts; ++j )
        {
            uint32_t dup = dupVerts[ j ];
            uint32_t dest = ( vertexRemap ) ? vertexRemap[ nVerts + j ] : uint32_t( nVerts + j );

            if ( dest == UNUSED32 )
            {
                // remap entry is unused
            }
            else if ( dup < nVerts && dest < newVerts )
            {
                sptr = reinterpret_cast<const uint8_t*>( vbin ) + dup * stride; 
                memcpy( dptr + dest * stride, sptr, stride ); 
            }
            else
                return false;
        }
    }

    return true;
}

#pragma warning(pop)

_Use_decl_annotations_
static Bool FinalizeVB( void* vb, size_t stride, size_t nVerts, const uint32_t* vertexRemap )
{
    if ( nVerts >= UINT32_MAX )
        return false;

    return SwapVertices( vb, stride, nVerts, nullptr, vertexRemap );
}


//-------------------------------------------------------------------------------------
// Applies a vertex remap and/or a vertex duplication set to a vertex buffer and
// point representatives
//-------------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning( disable : 6101 )

_Use_decl_annotations_
static Bool FinalizeVBAndPointReps( const void* vbin, size_t stride, size_t nVerts, const uint32_t* prin, 
                                const uint32_t* dupVerts, size_t nDupVerts, const uint32_t* vertexRemap, 
                                void* vbout, uint32_t* prout )
{
    if ( !vbin || !stride || !nVerts || !prin || !vbout || !prout )
        return false;

    if ( !dupVerts && !vertexRemap )
        return false;

    if ( dupVerts && !nDupVerts )
        return false;

    if ( !dupVerts && nDupVerts > 0 )
        return false;

    if ( nVerts >= UINT32_MAX )
        return false;

    if ( (uint64_t(nVerts) + uint64_t(nDupVerts) ) >= UINT32_MAX )
        return false;

    if ( vbin == vbout )
        return false;

    size_t newVerts = nVerts + nDupVerts;

    const uint8_t *sptr = reinterpret_cast<const uint8_t*>( vbin );
          uint8_t *dptr = reinterpret_cast<uint8_t*>( vbout );

#if DEBUG
    memset( vbout, 0, newVerts * stride );
#endif

    std__unique_ptr<uint32_t> pointRep( nVerts + nDupVerts );
    memcpy( pointRep.get(), prin, sizeof(uint32_t) * nVerts );
    for( size_t i = 0; i < nDupVerts; ++i )
    {
        pointRep[ i + nVerts ] = prin[ dupVerts[ i ] ];
    }
    
    if ( vertexRemap )
    {
        // clean up point reps for any removed vertices
        for( uint32_t i = 0; i < newVerts; ++i )
        {
            if ( vertexRemap[ i ] != UNUSED32 )
            {
                uint32_t old = pointRep[ i ];
                if ( ( old != UNUSED32 ) && ( vertexRemap[old] == UNUSED32 ) )
                {
                    pointRep[ i ] = i;

                    for( size_t k = (i+1); k < newVerts; ++k )
                    {
                        if ( pointRep[ k ] == old )
                            pointRep[ k ] = i;
                    }
                }
            }
        }
    }

    size_t j = 0;

    for( ; j < nVerts; ++j )
    {
        uint32_t dest = ( vertexRemap ) ? vertexRemap[ j ] : uint32_t(j);

        if ( dest == UNUSED32 )
        {
            // remap entry is unused
        }
        else if ( dest < newVerts )
        {
            memcpy( dptr + dest * stride, sptr, stride ); 

            uint32_t pr = pointRep[ j ];
            if ( pr < newVerts )
            {
                prout[ dest ] = ( vertexRemap ) ? vertexRemap[ pr ] : pr;
            }
        }
        else
            return false;

        sptr += stride;
    }

    if ( dupVerts )
    {
        for( size_t k = 0; k < nDupVerts; ++k )
        {
            uint32_t dup = dupVerts[ k ];
            uint32_t dest = ( vertexRemap ) ? vertexRemap[ nVerts + k ] : uint32_t( nVerts + k );

            if ( dest == UNUSED32 )
            {
                // remap entry is unused
            }
            else if ( dup < nVerts && dest < newVerts )
            {
                sptr = reinterpret_cast<const uint8_t*>( vbin ) + dup * stride; 
                memcpy( dptr + dest * stride, sptr, stride ); 

                uint32_t pr = pointRep[ nVerts + k ];
                if (pr < (nVerts + nDupVerts) )
                {
                    prout[ dest ] = ( vertexRemap ) ? vertexRemap[ pr ] : pr;
                }
            }
            else
                return false;
        }
    }

    return true;
}

#pragma warning(pop)

_Use_decl_annotations_
static Bool FinalizeVBAndPointReps( void* vb, size_t stride, size_t nVerts, uint32_t* pointRep, const uint32_t* vertexRemap )
{
    if ( nVerts >= UINT32_MAX )
        return false;

    if ( !pointRep || !vertexRemap )
        return false;

    // clean up point reps for any removed vertices
    for( uint32_t i = 0; i < nVerts; ++i )
    {
        if ( vertexRemap[ i ] != UNUSED32 )
        {
            uint32_t old = pointRep[ i ];
            if ( ( old != UNUSED32 ) && ( vertexRemap[old] == UNUSED32 ) )
            {
                pointRep[ i ] = i;

                for( size_t k = (i+1); k < nVerts; ++k )
                {
                    if ( pointRep[ k ] == old )
                        pointRep[ k ] = i;
                }
            }
        }
    }

    return SwapVertices( vb, stride, nVerts, pointRep, vertexRemap );
}


//-------------------------------------------------------------------------------------
template<class index_t>
class mesh_status
{
public:
    mesh_status() :
        mUnprocessed{},
        mFaceOffset(0),
        mFaceCount(0),
        mMaxSubset(0),
        mTotalFaces(0)
    {
    }

    Bool initialize( _In_reads_(nFaces*3) const index_t* indices, size_t nFaces,
                     _In_reads_(nFaces*3) const uint32_t* adjacency, _In_ const std__vector< std__pair<size_t,size_t> >& subsets )
    {
        if ( !indices || !nFaces || !adjacency || subsets.empty() )
            return false;

        // Convert adjacency to 'physical' adjacency
        mPhysicalNeighbors.reset( nFaces );
        if ( !mPhysicalNeighbors )
            return false;

#if DEBUG
        memset( mPhysicalNeighbors.get(), 0xcd, sizeof(neighborInfo) * nFaces );
#endif

        mFaceOffset = 0;
        mFaceCount = 0;
        mMaxSubset = 0;
        mTotalFaces = nFaces;

        for( const std__pair<size_t,size_t>* it = subsets.cbegin(); it != subsets.cend(); ++it )
        {
            if ( ( uint64_t(it->first) + uint64_t(it->second) ) >= UINT32_MAX )
                return false;

            if ( it->second > mMaxSubset )
            {
                mMaxSubset = it->second;
            }

            uint32_t faceOffset = uint32_t( it->first );
            uint32_t faceMax = uint32_t( it->first + it->second );

            for( uint32_t face = faceOffset; face < faceMax; ++face )
            {
                if ( face >= nFaces )
                    return false;

                index_t i0 = indices[ face*3 ];
                index_t i1 = indices[ face*3 + 1 ];
                index_t i2 = indices[ face*3 + 2 ];

                if ( i0 == index_t(-1)
                     || i1 == index_t(-1)
                     || i2 == index_t(-1)
                     || i0 == i1
                     || i0 == i2
                     || i1 == i2 )
                {
                    // unused and degenerate faces should not have neighbors
                    for( uint32_t point = 0; point < 3; ++point )
                    {
                        uint32_t k = adjacency[ face * 3 + point ];

                        if ( k != UNUSED32 )
                        {
                            if ( k >= nFaces )
                                return false;
 
                            if ( adjacency[ k*3 ] == face )
                                mPhysicalNeighbors[ k ].neighbors[ 0 ] = UNUSED32;

                            if ( adjacency[ k*3 + 1 ] == face )
                                mPhysicalNeighbors[ k ].neighbors[ 1 ] = UNUSED32;

                            if ( adjacency[ k*3 + 2 ] == face )
                                mPhysicalNeighbors[ k ].neighbors[ 2 ] = UNUSED32;
                        }

                        mPhysicalNeighbors[ face ].neighbors[ point ] = UNUSED32;
                    }
                }
                else
                {
                    for( uint32_t n = 0; n < 3; ++n )
                    {
                        uint32_t neighbor = adjacency[ face * 3 + n ];

                        if ( neighbor != UNUSED32 )
                        {
                            if ( ( neighbor < faceOffset ) || ( neighbor >= faceMax )
                                 || ( neighbor == adjacency[ face * 3 + ( ( n + 1 ) % 3 ) ] )
                                 || ( neighbor == adjacency[ face * 3 + ( ( n + 2 ) % 3 ) ] ) )
                            {
                                // Break links for any neighbors outside of our attribute set, and remove duplicate neighbors
                                neighbor = UNUSED32;
                            }
                            else
                            {
                                uint32_t edgeBack = find_edge<uint32_t>( &adjacency[ neighbor * 3 ], face );
                                if ( edgeBack < 3 )
                                {
                                    index_t p1 = indices[ face * 3 + n ];
                                    index_t p2 = indices[ face * 3 + ( ( n + 1 ) % 3 ) ];

                                    index_t pn1 = indices[ neighbor * 3 + edgeBack ];
                                    index_t pn2 = indices[ neighbor * 3 + ( ( edgeBack + 1 ) % 3 ) ];

                                    // if wedge not identical on shared edge, drop link
                                    if ( ( p1 != pn2 ) || ( p2 != pn1 ) )
                                    {
                                        neighbor = UNUSED32;
                                    }
                                }
                                else
                                {
                                    neighbor = UNUSED32;
                                }
                            }
                        }

                        mPhysicalNeighbors[ face ].neighbors[ n ] = neighbor;
                    }
                }
            }
        }
        
        if ( !mMaxSubset )
            return false;

        mListElements.reset( mMaxSubset );
        if ( !mListElements )
            return false;

        return true;
    }

    Bool setSubset( _In_reads_(nFaces*3) const index_t* indices, size_t nFaces, size_t faceOffset, size_t faceCount )
    {
        if ( !faceCount || !indices || !nFaces )
            return false;

        if ( faceCount > mMaxSubset )
            return false;

        if ( !mListElements )
            return false;

        if ( ( uint64_t(faceOffset) + uint64_t(faceCount) ) >= UINT32_MAX )
            return false;

        uint32_t faceMax = uint32_t( faceOffset + faceCount );

        if ( faceMax > nFaces )
            return false;

        mFaceOffset = faceOffset;
        mFaceCount = faceCount;

        mUnprocessed[0] = UNUSED32;
        mUnprocessed[1] = UNUSED32;
        mUnprocessed[2] = UNUSED32;
        mUnprocessed[3] = UNUSED32;

        for( uint32_t face = uint32_t( faceOffset ); face < faceMax; ++face )
        {
            index_t i0 = indices[ face*3 ];
            index_t i1 = indices[ face*3 + 1 ];
            index_t i2 = indices[ face*3 + 2 ];

            if ( i0 == index_t(-1)
                 || i1 == index_t(-1)
                 || i2 == index_t(-1) )
            {
                // filter out unused triangles
                continue;
            }

            uint32_t unprocessed = 0;

            for( uint32_t n = 0; n < 3; ++n )
            {
                if ( mPhysicalNeighbors[ face ].neighbors[ n ] != UNUSED32 )
                {
                    unprocessed += 1;

                    assert( mPhysicalNeighbors[ face ].neighbors[ n ] >= mFaceOffset );
                    assert( mPhysicalNeighbors[ face ].neighbors[ n ] < faceMax );
                }
            }

            uint32_t faceIndex = uint32_t( face - faceOffset );
            mListElements[ faceIndex ].processed = false;
            mListElements[ faceIndex ].unprocessed = unprocessed;

            push_front( faceIndex );
        }

        return true;
    }

    bool isprocessed( uint32_t face ) const 
    {
        assert( face < mTotalFaces );
        assert( ( face >= mFaceOffset ) || ( face < ( mFaceOffset + mFaceCount ) ) );
        return mListElements[ face - mFaceOffset ].processed;
    }

    uint32_t unprocessed_count( uint32_t face ) const
    {
        assert( face < mTotalFaces );
        assert( ( face >= mFaceOffset ) || ( face < ( mFaceOffset + mFaceCount ) ) );
        return mListElements[ face - mFaceOffset ].unprocessed;
    }

    uint32_t find_initial() const
    {
        for( size_t j = 0; j < 4; ++j )
        {
            if  ( mUnprocessed[j] != UNUSED32 )
                return uint32_t( mUnprocessed[ j ] + mFaceOffset );
        }

        return UNUSED32;
    }

    void mark( uint32_t face )
    {
        assert( face < mTotalFaces );
        assert( ( face >= mFaceOffset ) || ( face < ( mFaceOffset + mFaceCount ) ) );

        uint32_t faceIndex = uint32_t( face - mFaceOffset );

        assert( !mListElements[ faceIndex ].processed );
        mListElements[ faceIndex ].processed = true;

        remove( faceIndex );

        for( uint32_t n = 0; n < 3; ++n )
        {
            uint32_t neighbor = mPhysicalNeighbors[ face ].neighbors[ n ];
            if ( ( neighbor != UNUSED32 ) && !isprocessed( neighbor ) )
            {
                decrement( neighbor );
            }
        }
    }

    uint32_t find_next( uint32_t face ) const
    {
        assert( face < mTotalFaces );
        assert( ( face >= mFaceOffset ) || ( face < ( mFaceOffset + mFaceCount ) ) );

        uint32_t iret = 3;
        uint32_t minNeighbor = UNUSED32;
        uint32_t minNextNeighbor = 0;

        for( uint32_t n = 0; n < 3; ++n )
        {
            uint32_t neighbor = mPhysicalNeighbors[ face ].neighbors[ n ];

            if( ( neighbor == UNUSED32 ) || isprocessed( neighbor ) )
                continue;

            uint32_t unprocessed = unprocessed_count( neighbor );
            assert( unprocessed < 3 );

            uint32_t mintemp = UNUSED32;

            for( uint32_t nt = 0; nt < 3; ++nt )
            {
                uint32_t neighborTemp = mPhysicalNeighbors[ neighbor ].neighbors[ nt ];

                if( ( neighborTemp == UNUSED32 ) || isprocessed( neighborTemp ) )
                    continue;

                uint32_t next_count = unprocessed_count( neighborTemp );
                if ( next_count < mintemp )
                    mintemp = next_count;
            }

            if ( mintemp == UNUSED32 )
                mintemp = 0;

            if ( unprocessed < minNeighbor )
            {
                iret = n;
                minNeighbor = unprocessed;
                minNextNeighbor = mintemp;
            }
            else if ( ( unprocessed == minNeighbor ) && ( mintemp < minNextNeighbor ) )
            {
                iret = n;
                minNextNeighbor = mintemp;
            }
        }

        return iret;
    }

    const uint32_t get_neighbors( uint32_t face, uint32_t n ) const
    {
        assert( face < mTotalFaces );
        assert( n < 3 );
        return mPhysicalNeighbors[ face ].neighbors[ n ];
    }

    const uint32_t* get_neighborsPtr( uint32_t face ) const
    {
        assert( face < mTotalFaces );
        return &mPhysicalNeighbors[ face ].neighbors[ 0 ];
    }

private:
    void push_front( uint32_t faceIndex )
    {
        assert( faceIndex < mFaceCount );

        uint32_t unprocessed = mListElements[ faceIndex ].unprocessed;
    
        uint32_t head = mUnprocessed[ unprocessed ];
        mListElements[ faceIndex ].next = head;

        if ( head != UNUSED32 )
            mListElements[ head ].prev = faceIndex;

        mUnprocessed[ unprocessed ] = faceIndex;

        mListElements[ faceIndex ].prev = UNUSED32;
    }

    void remove( uint32_t faceIndex )
    {
        assert( faceIndex < mFaceCount );

        if ( mListElements[ faceIndex ].prev != UNUSED32 )
        {
            assert( mUnprocessed[ mListElements[ faceIndex ].unprocessed ] != faceIndex );

            uint32_t prev = mListElements[ faceIndex ].prev;
            uint32_t next = mListElements[ faceIndex ].next;

            mListElements[ prev ].next = next;

            if ( next != UNUSED32 )
            {
                mListElements[ next ].prev = prev;
            }
        }
        else
        {
            // remove head of the list
            assert( mUnprocessed[ mListElements[ faceIndex ].unprocessed ] == faceIndex );

            uint32_t unprocessed = mListElements[ faceIndex ].unprocessed;

            mUnprocessed[ unprocessed ] = mListElements[ faceIndex ].next;

            if ( mUnprocessed[ unprocessed ] != UNUSED32 )
            {
                mListElements[ mUnprocessed[ unprocessed ] ].prev = UNUSED32;
            }
        }

        mListElements[ faceIndex ].prev =
        mListElements[ faceIndex ].next = UNUSED32;
    }

    void decrement( uint32_t face )
    {
        assert( face < mTotalFaces );
        assert( ( face >= mFaceOffset ) || ( face < ( mFaceOffset + mFaceCount ) ) );
        assert( !isprocessed(face) );

        uint32_t faceIndex = uint32_t( face - mFaceOffset );

        assert( (mListElements[faceIndex].unprocessed >= 1) && (mListElements[faceIndex].unprocessed <= 3) );

        remove( faceIndex );

        mListElements[ faceIndex ].unprocessed -= 1;

        push_front( faceIndex );
    }

    struct neighborInfo
    {
        uint32_t neighbors[3];
    };

    struct listElement
    {
        bool        processed;
        uint32_t    unprocessed;
        uint32_t    prev;
        uint32_t    next;
    };

    uint32_t                      mUnprocessed[4];
    size_t                        mFaceOffset;
    size_t                        mFaceCount;
    size_t                        mMaxSubset;
    size_t                        mTotalFaces;
    std__unique_ptr<listElement>  mListElements;
    std__unique_ptr<neighborInfo> mPhysicalNeighbors;
};


//-------------------------------------------------------------------------------------
typedef std__pair<uint32_t,uint32_t> facecorner_t;

template<class index_t>
inline facecorner_t counterclockwise_corner( facecorner_t corner, mesh_status<index_t>& status )
{
    assert( corner.second != UNUSED32 );
    uint32_t edge = ( corner.second + 2 ) % 3;
    uint32_t neighbor = status.get_neighbors( corner.first, edge );
    uint32_t point = ( neighbor == UNUSED32 ) ? UNUSED32 : find_edge( status.get_neighborsPtr( neighbor ), corner.first );
    return facecorner_t( neighbor, point );
}


//-------------------------------------------------------------------------------------
class sim_vcache
{
public:
    sim_vcache() : mTail(0), mCacheSize(0) {}

    Bool initialize( uint32_t cacheSize )
    {
        if ( !cacheSize )
            return false;

        mFIFO.reset(cacheSize);
        if ( !mFIFO )
            return false;

        mCacheSize = cacheSize;

        clear();

        return true;
    }

    void clear()
    {
        assert( mFIFO != 0 );
        mTail = 0;
        memset( mFIFO.get(), 0xff, sizeof(uint32_t) * mCacheSize );
    }

    bool access( uint32_t vertex )
    {
        assert( vertex != UNUSED32 );
        assert( mFIFO != 0 );

        for( size_t ptr = 0; ptr < mCacheSize; ++ptr )
        {
            if ( mFIFO[ ptr ] == vertex )
            {
                return true;
            }
        }

        mFIFO[ mTail ] = vertex;
        mTail += 1;
        if ( mTail == mCacheSize )
            mTail = 0;

        return false;
    }

private:
    uint32_t                  mTail;
    uint32_t                  mCacheSize;
    std__unique_ptr<uint32_t> mFIFO;
};


//-------------------------------------------------------------------------------------
template<class index_t>
static Bool _StripReorder( _In_reads_(nFaces*3) const index_t* indices, _In_ size_t nFaces,
                       _In_reads_(nFaces*3) const uint32_t* adjacency,
                       _In_reads_opt_(nFaces) const uint32_t* attributes,
                       _Out_writes_(nFaces) uint32_t* faceRemap )
{
    std__vector< std__pair<size_t,size_t> > subsets; ComputeSubsets( subsets, attributes, nFaces );

    assert( !subsets.empty() );

    mesh_status<index_t> status;
    if(!status.initialize( indices, nFaces, adjacency, subsets ))return false;

    std__unique_ptr<uint32_t> faceRemapInverse(nFaces);
    if ( !faceRemapInverse )return false;

    memset( faceRemapInverse.get(), 0xff, sizeof(uint32_t) * nFaces );

    for( const std__pair<size_t,size_t>* it = subsets.cbegin(); it != subsets.cend(); ++it )
    {
        if(!status.setSubset( indices, nFaces, it->first, it->second ))return false;

        uint32_t curface = 0;

        for(;;)
        {
            uint32_t face = status.find_initial();
            if ( face == UNUSED32 )
                break;

            status.mark( face );

            uint32_t next = status.find_next( face );

            for(;;)
            {
                assert( face != UNUSED32 );
                faceRemapInverse[ face ] = uint32_t( curface + it->first );
                curface += 1;

                // if at end of strip, break out
                if ( next >= 3 )
                    break;

                face = status.get_neighbors( face, next );
                assert( face != UNUSED32 );

                status.mark( face );

                next = status.find_next( face );
            }
        }
    }

    // inverse remap
    memset( faceRemap, 0xff, sizeof(uint32_t) * nFaces );

    for( uint32_t j = 0; j < nFaces; ++j )
    {
        uint32_t f = faceRemapInverse[ j ];
        if ( f < nFaces )
        {
            faceRemap[ f ] = j;
        }
    }

    return true;
}


//-------------------------------------------------------------------------------------
template<class index_t>
static Bool _VertexCacheStripReorder( _In_reads_(nFaces*3) const index_t* indices, _In_ size_t nFaces,
                                  _In_reads_(nFaces*3) const uint32_t* adjacency,
                                  _In_reads_opt_(nFaces) const uint32_t* attributes,
                                  _Out_writes_(nFaces) uint32_t* faceRemap,
                                  uint32_t vertexCache, uint32_t restart )
{
    std__vector< std__pair<size_t,size_t> > subsets; ComputeSubsets( subsets, attributes, nFaces );

    assert( !subsets.empty() );

    mesh_status<index_t> status;
    if(!status.initialize( indices, nFaces, adjacency, subsets ))return false;

    sim_vcache vcache;
    if(!vcache.initialize( vertexCache ))return false;

    std__unique_ptr<uint32_t> faceRemapInverse( nFaces );
    if ( !faceRemapInverse )return false;

    memset( faceRemapInverse.get(), 0xff, sizeof(uint32_t) * nFaces );

    assert( vertexCache >= restart );
    uint32_t desired = vertexCache - restart;

    for( const std__pair<size_t,size_t>* it = subsets.cbegin(); it != subsets.cend(); ++it )
    {
        if(!status.setSubset( indices, nFaces, it->first, it->second ))return false;

        vcache.clear();

        uint32_t locnext = 0;
        facecorner_t nextCorner( UNUSED32, UNUSED32 );
        facecorner_t curCorner( UNUSED32, UNUSED32 );

        uint32_t curface = 0;

        for(;;)
        {
            assert( nextCorner.first == UNUSED32 );

            curCorner.first = status.find_initial();
            if ( curCorner.first == UNUSED32 )
                break;

            uint32_t n0 = status.get_neighbors( curCorner.first, 0 );
            if ( ( n0 != UNUSED32 ) && !status.isprocessed( n0 ) )
            {
                curCorner.second = 1;
            }
            else
            {
                uint32_t n1 = status.get_neighbors( curCorner.first, 1 );
                if ( ( n1 != UNUSED32 ) && !status.isprocessed( n1 ) )
                {
                    curCorner.second = 2;
                }
                else
                {
                    curCorner.second = 0;
                }
            }
             
            bool striprestart = false;
            for(;;)
            {
                assert( curCorner.first != UNUSED32 );
                assert( !status.isprocessed( curCorner.first ) );

                // Decision: either add a ring of faces or restart strip
                if ( nextCorner.first != UNUSED32 )
                {
                    uint32_t nf = 0;
                    for( facecorner_t temp = curCorner; ; )
                    {
                        facecorner_t next = counterclockwise_corner<index_t>( temp, status );
                        if ( ( next.first == UNUSED32 ) || status.isprocessed( next.first ) )
                            break;
                        ++nf;
                        temp = next;
                    }

                    if ( locnext + nf > desired )
                    {
                        // restart
                        if ( !status.isprocessed( nextCorner.first ) )
                        {
                            curCorner = nextCorner;
                        }

                        nextCorner.first = UNUSED32;
                    }
                }

                for(;;)
                {
                    assert( curCorner.first != UNUSED32 );
                    status.mark( curCorner.first );

                    faceRemapInverse[ curCorner.first ] = uint32_t( curface + it->first );
                    curface += 1;

                    assert( indices[ curCorner.first * 3 ] != index_t(-1) );
                    if ( !vcache.access( indices[ curCorner.first * 3 ] ) )
                        locnext += 1;

                    assert( indices[ curCorner.first * 3 + 1 ] != index_t(-1) );
                    if ( !vcache.access( indices[ curCorner.first * 3 + 1 ] ) )
                        locnext += 1;

                    assert( indices[ curCorner.first * 3 + 2 ] != index_t(-1) );
                    if ( !vcache.access( indices[ curCorner.first * 3 + 2 ] ) )
                        locnext += 1;

                    facecorner_t intCorner = counterclockwise_corner<index_t>( curCorner, status );
                    bool interiornei = ( intCorner.first != UNUSED32 ) && !status.isprocessed( intCorner.first );

                    facecorner_t extCorner = counterclockwise_corner<index_t>( facecorner_t( curCorner.first, ( curCorner.second + 2 ) % 3 ), status );
                    bool exteriornei = ( extCorner.first != UNUSED32 ) && !status.isprocessed( extCorner.first );

                    if ( interiornei )
                    {
                        if ( exteriornei )
                        {
                            if ( nextCorner.first == UNUSED32 )
                            {
                                nextCorner = extCorner;
                                locnext = 0;
                            }
                        }
                        curCorner = intCorner;
                    }
                    else if ( exteriornei )
                    {
                        curCorner = extCorner;
                        break;
                    }
                    else
                    {
                        curCorner = nextCorner;
                        nextCorner.first = UNUSED32;

                        if ( ( curCorner.first == UNUSED32 ) || status.isprocessed( curCorner.first ) )
                        {
                            striprestart = true;
                            break;
                        }
                    }
                }

                if ( striprestart )
                    break;
            }
        }
    }

    // inverse remap
    memset( faceRemap, 0xff, sizeof(uint32_t) * nFaces );

    for( uint32_t j = 0; j < nFaces; ++j )
    {
        uint32_t f = faceRemapInverse[ j ];
        if ( f < nFaces )
        {
            faceRemap[ f ] = j;
        }
    }

    return true;
}


//-------------------------------------------------------------------------------------
template<class index_t>
static Bool _OptimizeVertices( const index_t* indices, size_t nFaces, size_t nVerts, uint32_t* vertexRemap )
{
    if ( !indices || !nFaces || !nVerts || !vertexRemap )
        return false;

    if ( nVerts >= index_t(-1) )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    std__unique_ptr<uint32_t> tempRemap( nVerts );
    if ( !tempRemap )
        return false;

    memset( tempRemap.get(), 0xff, sizeof(uint32_t) * nVerts );

    uint32_t curvertex = 0;
    for( size_t j = 0; j < (nFaces * 3); ++j )
    {
        index_t curindex = indices[ j ];
        if ( curindex == index_t(-1) )
            continue;

        if ( curindex >= nVerts )
            return false;

        if ( tempRemap[ curindex ] == UNUSED32 )
        {
            tempRemap[ curindex ] = curvertex;
            ++curvertex;
        }
    }

    // inverse lookup
    memset( vertexRemap, 0xff, sizeof(uint32_t) * nVerts );

    for( uint32_t j = 0; j < nVerts; ++j )
    {
        uint32_t vertindex = tempRemap[ j ];
        if ( vertindex != UNUSED32 )
        {
            if ( vertindex >= nVerts )
                return false;

            vertexRemap[ vertindex ] = j;
        }
    }

    return true;
}

};

namespace DirectX
{

//=====================================================================================
// Entry-points
//=====================================================================================

//-------------------------------------------------------------------------------------
/*_Use_decl_annotations_
HRESULT AttributeSort( size_t nFaces, uint32_t* attributes, uint32_t* faceRemap )
{
    if ( !nFaces || !attributes || !faceRemap )
        return E_INVALIDARG;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return HRESULT_FROM_WIN32( ERROR_ARITHMETIC_OVERFLOW );

    typedef std__pair<uint32_t,uint32_t> intpair_t;

    std__vector<intpair_t> list;
    list.reserve( nFaces );
    for( uint32_t j=0; j < nFaces; ++j )
    {
        list.emplace_back( intpair_t( attributes[ j ], j ) );
    }

    std::stable_sort( list.begin(), list.end(), [](const intpair_t& a, const intpair_t& b ) -> bool
                                                {
                                                    return (a.first < b.first);
                                                });

    auto it = list.begin();
    for( uint32_t j = 0; j < nFaces; ++j, ++it )
    {
        attributes[ j ] = it->first;
        faceRemap[ j ] = it->second;
    }

    return S_OK;
}*/


//-------------------------------------------------------------------------------------
_Use_decl_annotations_
static Bool OptimizeFaces( const uint16_t* indices, size_t nFaces, const uint32_t* adjacency,
                       uint32_t* faceRemap, uint32_t vertexCache=OPTFACES_V_DEFAULT, uint32_t restart=OPTFACES_R_DEFAULT )
{
    if ( !indices || !nFaces || !adjacency || !faceRemap )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    if( vertexCache == OPTFACES_V_STRIPORDER )
    {
        return _StripReorder<uint16_t>( indices, nFaces, adjacency, nullptr, faceRemap );
    }
    else
    {
        if ( restart > vertexCache )
            return false;

        return _VertexCacheStripReorder<uint16_t>( indices, nFaces, adjacency, nullptr, faceRemap, vertexCache, restart );
    }
}

_Use_decl_annotations_
static Bool OptimizeFaces( const uint32_t* indices, size_t nFaces, const uint32_t* adjacency,
                       uint32_t* faceRemap, uint32_t vertexCache=OPTFACES_V_DEFAULT, uint32_t restart=OPTFACES_R_DEFAULT )
{
    if ( !indices || !nFaces || !adjacency || !faceRemap )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    if( vertexCache == OPTFACES_V_STRIPORDER )
    {
        return _StripReorder<uint32_t>( indices, nFaces, adjacency, nullptr, faceRemap );
    }
    else
    {
        if ( restart > vertexCache )
            return false;

        return _VertexCacheStripReorder<uint32_t>( indices, nFaces, adjacency, nullptr, faceRemap, vertexCache, restart );
    }
}


//-------------------------------------------------------------------------------------
_Use_decl_annotations_
static Bool OptimizeFacesEx( const uint16_t* indices, size_t nFaces, const uint32_t* adjacency, const uint32_t* attributes,
                         uint32_t* faceRemap, uint32_t vertexCache=OPTFACES_V_DEFAULT, uint32_t restart=OPTFACES_R_DEFAULT )
{
    if ( !indices || !nFaces || !adjacency || !attributes || !faceRemap )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    if( vertexCache == OPTFACES_V_STRIPORDER )
    {
        return _StripReorder<uint16_t>( indices, nFaces, adjacency, attributes, faceRemap );
    }
    else
    {
        if ( restart > vertexCache )
            return false;

        return _VertexCacheStripReorder<uint16_t>( indices, nFaces, adjacency, attributes, faceRemap, vertexCache, restart );
    }
}

_Use_decl_annotations_
static Bool OptimizeFacesEx( const uint32_t* indices, size_t nFaces, const uint32_t* adjacency, const uint32_t* attributes,
                         uint32_t* faceRemap, uint32_t vertexCache=OPTFACES_V_DEFAULT, uint32_t restart=OPTFACES_R_DEFAULT )
{
    if ( !indices || !nFaces || !adjacency || !attributes || !faceRemap )
        return false;

    if ( ( uint64_t(nFaces) * 3 ) >= UINT32_MAX )
        return false;

    if( vertexCache == OPTFACES_V_STRIPORDER )
    {
        return _StripReorder<uint32_t>( indices, nFaces, adjacency, attributes, faceRemap );
    }
    else
    {
        if ( restart > vertexCache )
            return false;

        return _VertexCacheStripReorder<uint32_t>( indices, nFaces, adjacency, attributes, faceRemap, vertexCache, restart );
    }
}


//-------------------------------------------------------------------------------------
_Use_decl_annotations_
static Bool OptimizeVertices( const uint16_t* indices, size_t nFaces, size_t nVerts, uint32_t* vertexRemap )
{
    return _OptimizeVertices<uint16_t>( indices, nFaces, nVerts, vertexRemap );
}

_Use_decl_annotations_
static Bool OptimizeVertices( const uint32_t* indices, size_t nFaces, size_t nVerts, uint32_t* vertexRemap )
{
    return _OptimizeVertices<uint32_t>( indices, nFaces, nVerts, vertexRemap );
}

} // namespace DirectX
