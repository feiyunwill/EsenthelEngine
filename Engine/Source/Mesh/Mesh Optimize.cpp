/******************************************************************************/
#include "stdafx.h"
/******************************************************************************
             -----FPS------   optimizeTimeSeconds
Method       GL   DX9  DX10+
DirectX           31.1 25.6          0.014
Forsyth           29.1 23.6          0.028
ForsythSorted          23.4          0.030
Forsyth2               23.2          0.022
Tipsify                20.9          0.005
/******************************************************************************/
#define VTX_CACHE_SIZE 64

#define METHOD_DIRECTX        0
#define METHOD_FORSYTH        1
#define METHOD_FORSYTH_SORTED 2
#define METHOD_FORSYTH2       3
#define METHOD_TIPSIFY        4
#define METHOD_TEST           5
#define METHOD                METHOD_DIRECTX

#if METHOD==METHOD_DIRECTX || METHOD==METHOD_TEST
   #if DEBUG
      #undef NDEBUG
   #endif
   #include "../../../../ThirdPartyLibs/begin.h"
   #include "../../../../ThirdPartyLibs/DirectXMath/include.h"
   #include "../../../../ThirdPartyLibs/DirectXMesh/DirectXMeshOptimize.cpp"
   #include "../../../../ThirdPartyLibs/end.h"
#endif
/******************************************************************************/
#if METHOD==METHOD_FORSYTH || METHOD==METHOD_TEST
namespace Forsyth
{
// This is an implementation of Tom Forsyth's "Linear-Speed Vertex Cache Optimization" algorithm as described here:
// http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html
// This code was authored and released into the public domain by Adrian Stone (stone@gameangst.com)

   typedef Int Index;
   static const Index IndexMax=INT_MAX;

   // code for computing vertex score was taken, as much as possible directly from the original publication
   static Flt ComputeVertexCacheScore(int cachePosition, UInt vertexCacheSize)
   {
      const Flt FindVertexScore_CacheDecayPower=1.5f;
      const Flt FindVertexScore_LastTriScore   =0.75f;

      Flt score=0;
      if(cachePosition>=0) // <0 Vertex is not in FIFO cache-no score, otherwise:
      {
         if(cachePosition<3)
         {
            // This vertex was used in the last triangle, so it has a fixed score, whichever of the three it's in.
            // Otherwise, you can get very different answers depending on whether you add the triangle 1,2,3 or 3,1,2-which is silly.
            score=FindVertexScore_LastTriScore;
         }else
         {
            DEBUG_ASSERT(cachePosition<vertexCacheSize, "cache opt");
            // Points for being high in the cache.
            const Flt scaler=1.0f/(vertexCacheSize-3);
            score=1.0f-(cachePosition-3)*scaler;
            score=Pow(score, FindVertexScore_CacheDecayPower);
         }
      }
      return score;
   }

   static Flt ComputeVertexValenceScore(UInt numActiveFaces)
   {
      const Flt FindVertexScore_ValenceBoostScale=2.0f;
      const Flt FindVertexScore_ValenceBoostPower=0.5f;

      // Bonus points for having a low number of tris still to use the vert, so we get rid of lone verts quickly.
      Flt valenceBoost=Pow((Flt)numActiveFaces, -FindVertexScore_ValenceBoostPower);
      return FindVertexScore_ValenceBoostScale*valenceBoost;
   }

   static const UInt kMaxVertexCacheSize=VTX_CACHE_SIZE;
   static const UInt kMaxPrecomputedVertexValenceScores=VTX_CACHE_SIZE;
   static Flt s_vertexCacheScores  [kMaxVertexCacheSize+1][kMaxVertexCacheSize];
   static Flt s_vertexValenceScores[kMaxPrecomputedVertexValenceScores];

   static Bool ComputeVertexScores()
   {
      for(UInt cacheSize=0; cacheSize<=kMaxVertexCacheSize; ++cacheSize)
      for(UInt cachePos =0; cachePos <           cacheSize; ++cachePos )
         s_vertexCacheScores[cacheSize][cachePos]=ComputeVertexCacheScore(cachePos, cacheSize);

      for(UInt valence=0; valence<kMaxPrecomputedVertexValenceScores; ++valence)
         s_vertexValenceScores[valence]=ComputeVertexValenceScore(valence);

      return true;
   }

 //static inline Flt FindVertexCacheScore  (UInt cachePosition, UInt maxSizeVertexCache) {return s_vertexCacheScores  [maxSizeVertexCache][cachePosition];}
 //static inline Flt FindVertexValenceScore(UInt numActiveTris                         ) {return s_vertexValenceScores[numActiveTris];}

   static Flt FindVertexScore(UInt numActiveFaces, UInt cachePosition, UInt vertexCacheSize)
   {
      if(!numActiveFaces)return -1; // No tri needs this vertex

      Flt score=0;
      if(cachePosition<vertexCacheSize)score+=s_vertexCacheScores[vertexCacheSize][cachePosition];

      if(numActiveFaces<kMaxPrecomputedVertexValenceScores)score+=s_vertexValenceScores    [numActiveFaces];
      else                                                 score+=ComputeVertexValenceScore(numActiveFaces);

      return score;
   }

   struct OptimizeVertexData
   {
      Flt   score;
      UInt  activeFaceListStart, activeFaceListSize;
      Index cachePos0, cachePos1;

      OptimizeVertexData() : score(0), activeFaceListStart(0), activeFaceListSize(0), cachePos0(0), cachePos1(0) {}
   };

   static void OptimizeFaces(const Index *indexList, UInt indexCount, UInt vertexCount, Index *newIndexList, UInt lruCacheSize)
   {
      DEBUG_ASSERT(lruCacheSize<=kMaxVertexCacheSize, "cache opt"); // these codes assume that cache size is no greater than kMaxVertexCacheSize
      Mems<OptimizeVertexData> vertexDataList; vertexDataList.setNum(vertexCount);

      // compute face count per vertex
      for(UInt i=0; i<indexCount; ++i)
      {
         Index index=indexList[i];
         DEBUG_ASSERT(index<vertexCount, "cache opt");
         OptimizeVertexData &vertexData=vertexDataList[index];
         vertexData.activeFaceListSize++;
      }

      Memc<UInt> activeFaceList;

      const Index kEvictedCacheIndex=IndexMax;

      {
         // allocate face list per vertex
         UInt curActiveFaceListPos=0;
         for(UInt i=0; i<vertexCount; ++i)
         {
            OptimizeVertexData &vertexData=vertexDataList[i];
            vertexData.cachePos0=kEvictedCacheIndex;
            vertexData.cachePos1=kEvictedCacheIndex;
            vertexData.activeFaceListStart=curActiveFaceListPos;
            curActiveFaceListPos+=vertexData.activeFaceListSize;
            vertexData.score=FindVertexScore(vertexData.activeFaceListSize, vertexData.cachePos0, lruCacheSize);
            vertexData.activeFaceListSize=0;
         }
         activeFaceList.setNum(curActiveFaceListPos);
      }

      // fill out face list per vertex
      for(UInt i=0; i<indexCount; i+=3)
      for(UInt j=0; j<3; ++j)
      {
         Index index=indexList[i+j];
         OptimizeVertexData &vertexData=vertexDataList[index];
         activeFaceList[vertexData.activeFaceListStart + vertexData.activeFaceListSize]=i;
         vertexData.activeFaceListSize++;
      }

      Mems<Bool> processedFaceList; processedFaceList.setNumZero(indexCount);

      Index  vertexCacheBuffer[(kMaxVertexCacheSize+3)*2],
            *cache0=vertexCacheBuffer,
            *cache1=vertexCacheBuffer+(kMaxVertexCacheSize+3),
             entriesInCache0=0;
      UInt   bestFace = 0;
      Flt    bestScore=-1;

      const Flt maxValenceScore=FindVertexScore(1, kEvictedCacheIndex, lruCacheSize)*3;

      for(UInt i=0; i<indexCount; i+=3)
      {
         if(bestScore<0)
         {
            // no verts in the cache are used by any unprocessed faces so search all unprocessed faces for a new starting point
            for(UInt j=0; j<indexCount; j+=3)
            {
               if(!processedFaceList[j])
               {
                  UInt face=j;
                  Flt  faceScore=0;
                  for(UInt k=0; k<3; ++k)
                  {
                     Index index=indexList[face+k];
                     OptimizeVertexData &vertexData=vertexDataList[index];
                     DEBUG_ASSERT(vertexData.activeFaceListSize>0, "cache opt");
                     DEBUG_ASSERT(vertexData.cachePos0>=lruCacheSize, "cache opt");
                     faceScore+=vertexData.score;
                  }
                  if(faceScore>bestScore)
                  {
                     bestScore=faceScore;
                     bestFace =face;

                     DEBUG_ASSERT(bestScore <= maxValenceScore, "cache opt");
                     if(bestScore>=maxValenceScore)break;
                  }
               }
            }
            DEBUG_ASSERT(bestScore>=0, "cache opt");
         }

         processedFaceList[bestFace]=1;
         Index entriesInCache1=0;

         // add bestFace to LRU cache and to newIndexList
         for(UInt v=0; v<3; ++v)
         {
            Index index=indexList[bestFace+v];
            newIndexList[i+v]=index;

            OptimizeVertexData &vertexData=vertexDataList[index];

            if(vertexData.cachePos1>=entriesInCache1)
            {
               vertexData.cachePos1=entriesInCache1;
               cache1[entriesInCache1++]=index;

               if(vertexData.activeFaceListSize==1)
               {
                  --vertexData.activeFaceListSize;
                  continue;
               }
            }

            DEBUG_ASSERT(vertexData.activeFaceListSize>0, "cache opt");
            REP(vertexData.activeFaceListSize)
            {
               if(activeFaceList[vertexData.activeFaceListStart+i]==bestFace)
               {
                  Swap(activeFaceList[vertexData.activeFaceListStart+i], activeFaceList[vertexData.activeFaceListStart+vertexData.activeFaceListSize-1]);
                  vertexData.activeFaceListSize--;
                  goto found;
               }
            }
            DEBUG_ASSERT(false, "cache opt");
         found:;
            vertexData.score=FindVertexScore(vertexData.activeFaceListSize, vertexData.cachePos1, lruCacheSize);
         }

         // move the rest of the old verts in the cache down and compute their new scores
         for(UInt c0=0; c0<entriesInCache0; ++c0)
         {
            Index index=cache0[c0];
            OptimizeVertexData &vertexData=vertexDataList[index];

            if(vertexData.cachePos1>=entriesInCache1)
            {
               vertexData.cachePos1=entriesInCache1;
               cache1[entriesInCache1++]=index;
               vertexData.score=FindVertexScore(vertexData.activeFaceListSize, vertexData.cachePos1, lruCacheSize);
            }
         }

         // find the best scoring triangle in the current cache (including up to 3 that were just evicted)
         bestScore=-1;
         for(UInt c1=0; c1<entriesInCache1; ++c1)
         {
            Index index=cache1[c1];
            OptimizeVertexData &vertexData=vertexDataList[index];
            vertexData.cachePos0=vertexData.cachePos1;
            vertexData.cachePos1=kEvictedCacheIndex;
            for(UInt j=0; j<vertexData.activeFaceListSize; ++j)
            {
               UInt face=activeFaceList[vertexData.activeFaceListStart+j];
               Flt  faceScore=0;
               for(UInt v=0; v<3; v++)
               {
                  Index faceIndex=indexList[face+v];
                  OptimizeVertexData &faceVertexData=vertexDataList[faceIndex];
                  faceScore+=faceVertexData.score;
               }
               if(faceScore>bestScore)
               {
                  bestScore=faceScore;
                  bestFace =face;
               }
            }
         }

         Swap(cache0, cache1);
         entriesInCache0=Min(entriesInCache1, lruCacheSize);
      }
   }
} // Forsyth
#endif
/******************************************************************************/
#if METHOD==METHOD_FORSYTH_SORTED || METHOD==METHOD_TEST
namespace ForsythSorted
{
    namespace
    {
        // code for computing vertex score was taken, as much as possible
        // directly from the original publication.
        float ComputeVertexCacheScore(int cachePosition, uint32_t vertexCacheSize)
        {
            const float FindVertexScore_CacheDecayPower = 1.5f;
            const float FindVertexScore_LastTriScore = 0.75f;

            float score = 0.0f;
            if ( cachePosition < 0 )
            {
                // Vertex is not in FIFO cache - no score.
            }
            else
            {
                if ( cachePosition < 3 )
                {
                    // This vertex was used in the last triangle,
                    // so it has a fixed score, whichever of the three
                    // it's in. Otherwise, you can get very different
                    // answers depending on whether you add
                    // the triangle 1,2,3 or 3,1,2 - which is silly.
                    score = FindVertexScore_LastTriScore;
                }
                else
                {
                    assert ( cachePosition < int(vertexCacheSize) );
                    // Points for being high in the cache.
                    const float scaler = 1.0f / ( vertexCacheSize - 3 );
                    score = 1.0f - ( cachePosition - 3 ) * scaler;
                    score = powf ( score, FindVertexScore_CacheDecayPower );
                }
            }

            return score;
        }

        float ComputeVertexValenceScore(uint32_t numActiveFaces)
        {
            const float FindVertexScore_ValenceBoostScale = 2.0f;
            const float FindVertexScore_ValenceBoostPower = 0.5f;

            float score = 0.f;

            // Bonus points for having a low number of tris still to
            // use the vert, so we get rid of lone verts quickly.
            float valenceBoost = powf ( static_cast<float>(numActiveFaces),
                -FindVertexScore_ValenceBoostPower );
            score += FindVertexScore_ValenceBoostScale * valenceBoost;

            return score;
        }


        enum {kMaxVertexCacheSize = VTX_CACHE_SIZE};
        enum {kMaxPrecomputedVertexValenceScores = VTX_CACHE_SIZE};
        float s_vertexCacheScores[kMaxVertexCacheSize+1][kMaxVertexCacheSize];
        float s_vertexValenceScores[kMaxPrecomputedVertexValenceScores];

        bool ComputeVertexScores()
        {
            for (uint32_t cacheSize=0; cacheSize<=kMaxVertexCacheSize; ++cacheSize)
            {
                for (uint32_t cachePos=0; cachePos<cacheSize; ++cachePos)
                {
                    s_vertexCacheScores[cacheSize][cachePos] = ComputeVertexCacheScore(cachePos, cacheSize);
                }
            }

            for (uint32_t valence=0; valence<kMaxPrecomputedVertexValenceScores; ++valence)
            {
                s_vertexValenceScores[valence] = ComputeVertexValenceScore(valence);
            }

            return true;
        }

        inline float FindVertexCacheScore(uint32_t cachePosition, uint32_t maxSizeVertexCache)
        {
            return s_vertexCacheScores[maxSizeVertexCache][cachePosition];
        }

        inline float FindVertexValenceScore(uint32_t numActiveTris)
        {
            return s_vertexValenceScores[numActiveTris];
        }

        float FindVertexScore(uint32_t numActiveFaces, uint32_t cachePosition, uint32_t vertexCacheSize)
        {
            if ( numActiveFaces == 0 )
            {
                // No tri needs this vertex!
                return -1.0f;
            }

            float score = 0.f;
            if (cachePosition < vertexCacheSize)
            {
                score += s_vertexCacheScores[vertexCacheSize][cachePosition];
            }

            if (numActiveFaces < kMaxPrecomputedVertexValenceScores)
            {
                score += s_vertexValenceScores[numActiveFaces];
            }
            else
            {
                score += ComputeVertexValenceScore(numActiveFaces);
            }

            return score;
        }

        template <typename IndexType>
        struct OptimizeVertexData
        {
            float   score;
            uint32_t    activeFaceListStart;
            uint32_t    activeFaceListSize;
            IndexType  cachePos0;
            IndexType  cachePos1;
            OptimizeVertexData() : score(0.f), activeFaceListStart(0), activeFaceListSize(0), cachePos0(0), cachePos1(0) { }
        };
    }

    template <typename TYPE, typename IndexType>
    struct IndexSortCompareIndexed
    {
        const IndexType *_indexData;

        IndexSortCompareIndexed(const IndexType *indexData)
            : _indexData(indexData)
        {
        }

        bool operator()(TYPE a, TYPE b) const
        {
            IndexType indexA = _indexData[a];
            IndexType indexB = _indexData[b];

            if (indexA < indexB)
                return true;
            return false;
        }
    };

    template <typename TYPE, typename IndexType>
    struct FaceValenceSort
    {
        const OptimizeVertexData<IndexType> *_vertexData;

        FaceValenceSort(const OptimizeVertexData<IndexType> *vertexData)
            : _vertexData(vertexData)
        {
        }

        bool operator()(TYPE a, TYPE b) const
        {
            const OptimizeVertexData<IndexType> *vA0 = _vertexData + a * 3 + 0;
            const OptimizeVertexData<IndexType> *vA1 = _vertexData + a * 3 + 1;
            const OptimizeVertexData<IndexType> *vA2 = _vertexData + a * 3 + 2;
            const OptimizeVertexData<IndexType> *vB0 = _vertexData + b * 3 + 0;
            const OptimizeVertexData<IndexType> *vB1 = _vertexData + b * 3 + 1;
            const OptimizeVertexData<IndexType> *vB2 = _vertexData + b * 3 + 2;

            int aValence = vA0->activeFaceListSize + vA1->activeFaceListSize + vA2->activeFaceListSize;
            int bValence = vB0->activeFaceListSize + vB1->activeFaceListSize + vB2->activeFaceListSize;

            // higher scoring faces are those with lower valence totals
            // reverse sort (reverse of reverse)
            if (aValence < bValence)
                return true;
            return false;
        }
    };

    //-----------------------------------------------------------------------------
    //  OptimizeFaces
    //-----------------------------------------------------------------------------
    //  Parameters:
    //      indexList
    //          input index list
    //      indexCount
    //          the number of indices in the list
    //      vertexCount
    //          the largest index value in indexList
    //      newIndexList
    //          a pointer to a preallocated buffer the same size as indexList to
    //          hold the optimized index list
    //      lruCacheSize
    //          the size of the simulated post-transform cache (max:VTX_CACHE_SIZE)
    //-----------------------------------------------------------------------------
    template <typename IndexType>
    void OptimizeFaces(const IndexType *indexList, uint32_t indexCount, IndexType *newIndexList, uint16_t lruCacheSize)
    {
        OptimizeVertexData<IndexType> *vertexDataList = new OptimizeVertexData<IndexType> [indexCount]; // upper bounds on size is indexCount
        IndexType *vertexRemap = new IndexType [indexCount];
        uint32_t *activeFaceList = new uint32_t [indexCount];

        uint32_t faceCount = indexCount / 3;
        uint8_t *processedFaceList = new uint8_t [faceCount];
        memset(processedFaceList, 0, sizeof(uint8_t) * faceCount);
        unsigned int *faceSorted = new unsigned int [faceCount];
        unsigned int *faceReverseLookup = new unsigned int [faceCount];

        // build the vertex remap table
        unsigned int uniqueVertexCount = 0;
        {
            typedef IndexSortCompareIndexed<unsigned int, IndexType> indexSorter;
            unsigned int *indexSorted = new unsigned int [indexCount];

            for (unsigned int i = 0; i < indexCount; i++)
            {
                indexSorted[i] = i;
            }

            indexSorter sortFunc(indexList);
            std::sort(indexSorted, indexSorted + indexCount, sortFunc);

            for (unsigned int i = 0; i < indexCount; i++)
            {
                if (i == 0
                    || sortFunc(indexSorted[i - 1], indexSorted[i]))
                {
                    // it's not a duplicate
                    vertexRemap[indexSorted[i]] = uniqueVertexCount;
                    uniqueVertexCount++;
                }
                else
                {
                    vertexRemap[indexSorted[i]] = vertexRemap[indexSorted[i - 1]];
                }
            }

            delete [] indexSorted;
        }

        // compute face count per vertex
        for (uint32_t i=0; i<indexCount; ++i)
        {
            OptimizeVertexData<IndexType>& vertexData = vertexDataList[vertexRemap[i]];
            vertexData.activeFaceListSize++;
        }

        const IndexType kEvictedCacheIndex = std::numeric_limits<IndexType>::max();
        {
            // allocate face list per vertex
            uint32_t curActiveFaceListPos = 0;
            for (uint32_t i = 0; i < uniqueVertexCount; ++i)
            {
                OptimizeVertexData<IndexType>& vertexData = vertexDataList[i];
                vertexData.cachePos0 = kEvictedCacheIndex;
                vertexData.cachePos1 = kEvictedCacheIndex;
                vertexData.activeFaceListStart = curActiveFaceListPos;
                curActiveFaceListPos += vertexData.activeFaceListSize;
                vertexData.score = FindVertexScore(vertexData.activeFaceListSize, vertexData.cachePos0, lruCacheSize);
                vertexData.activeFaceListSize = 0;
            }
            assert(curActiveFaceListPos == indexCount);
        }

        // sort unprocessed faces by highest score
        for (uint32_t f = 0; f < faceCount; f++)
        {
            faceSorted[f] = f;
        }
        FaceValenceSort<unsigned int, IndexType> faceValenceSort(vertexDataList);
        std::sort(faceSorted, faceSorted + faceCount, faceValenceSort);
        for (uint32_t f = 0; f < faceCount; f++)
        {
            faceReverseLookup[faceSorted[f]] = f;
        }

        // fill out face list per vertex
        for (uint32_t i=0; i<indexCount; i+=3)
        {
            for (uint32_t j=0; j<3; ++j)
            {
                OptimizeVertexData<IndexType>& vertexData = vertexDataList[vertexRemap[i + j]];
                activeFaceList[vertexData.activeFaceListStart + vertexData.activeFaceListSize] = i;
                vertexData.activeFaceListSize++;
            }
        }

        IndexType vertexCacheBuffer[(kMaxVertexCacheSize+3)*2];
        IndexType *cache0 = vertexCacheBuffer;
        IndexType *cache1 = vertexCacheBuffer+(kMaxVertexCacheSize+3);
        IndexType entriesInCache0 = 0;

        uint32_t bestFace = 0;
        float bestScore = -1.f;

        const float maxValenceScore = FindVertexScore(1, kEvictedCacheIndex, lruCacheSize) * 3.f;

        unsigned int nextBestFace = 0;
        for (uint32_t i = 0; i < indexCount; i += 3)
        {
            if (bestScore < 0.f)
            {
                // no verts in the cache are used by any unprocessed faces so
                // search all unprocessed faces for a new starting point
                for (; nextBestFace < faceCount; nextBestFace++)
                {
                    unsigned int faceIndex = faceSorted[nextBestFace];
                    if (processedFaceList[faceIndex] == 0)
                    {
                        uint32_t face = faceIndex * 3;
                        float faceScore = 0.f;
                        for (uint32_t k=0; k<3; ++k)
                        {
                            //assert(vertexData.activeFaceListSize > 0);
                            //assert(vertexData.cachePos0 >= lruCacheSize);

                            float vertexScore = vertexDataList[vertexRemap[face + k]].score;
                            faceScore += vertexScore; 
                        }

                        bestScore = faceScore;
                        bestFace = face;

                        nextBestFace++;
                        break; // we're searching a pre-sorted list, first one we find will be the best
                    }
                }
                assert(bestScore >= 0.f);
            }

            processedFaceList[bestFace / 3] = 1;
            uint16_t entriesInCache1 = 0;

            // add bestFace to LRU cache and to newIndexList
            for (uint32_t v = 0; v < 3; ++v)
            {
                IndexType index = indexList[bestFace+v];
                newIndexList[i+v] = index;

                OptimizeVertexData<IndexType>& vertexData = vertexDataList[vertexRemap[bestFace + v]];

                if (vertexData.cachePos1 >= entriesInCache1)
                {
                    vertexData.cachePos1 = entriesInCache1;
                    cache1[entriesInCache1++] = vertexRemap[bestFace + v];

                    if (vertexData.activeFaceListSize == 1)
                    {
                        --vertexData.activeFaceListSize;
                        continue;
                    }
                }

                assert(vertexData.activeFaceListSize > 0);
                uint32_t *begin = activeFaceList + vertexData.activeFaceListStart;
                uint32_t *end = activeFaceList + (vertexData.activeFaceListStart + vertexData.activeFaceListSize);
                uint32_t *it = std::find(begin, end, bestFace);
                assert(it != end);
                std::swap(*it, *(end-1));
                --vertexData.activeFaceListSize;
                vertexData.score = FindVertexScore(vertexData.activeFaceListSize, vertexData.cachePos1, lruCacheSize);

                // need to re-sort the faces that use this vertex, as their score will change due to activeFaceListSize shrinking
                for (uint32_t *fi = begin; fi != end - 1; ++fi)
                {
                    unsigned int faceIndex = *fi / 3;

                    unsigned int n = faceReverseLookup[faceIndex];
                    assert(faceSorted[n] == faceIndex);

                    // found it, now move it up
                    while (n > 0)
                    {
                        if (faceValenceSort(n, n - 1))
                        {
                            faceReverseLookup[faceSorted[n]] = n - 1;
                            faceReverseLookup[faceSorted[n - 1]] = n;
                            std::swap(faceSorted[n], faceSorted[n - 1]);
                            n--;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }

            // move the rest of the old verts in the cache down and compute their new scores
            for (uint32_t c0 = 0; c0 < entriesInCache0; ++c0)
            {
                OptimizeVertexData<IndexType>& vertexData = vertexDataList[cache0[c0]];

                if (vertexData.cachePos1 >= entriesInCache1)
                {
                    vertexData.cachePos1 = entriesInCache1;
                    cache1[entriesInCache1++] = cache0[c0];
                    vertexData.score = FindVertexScore(vertexData.activeFaceListSize, vertexData.cachePos1, lruCacheSize);
                    // don't need to re-sort this vertex... once it gets out of the cache, it'll have its original score
                }
            }

            // find the best scoring triangle in the current cache (including up to 3 that were just evicted)
            bestScore = -1.f;
            for (uint32_t c1 = 0; c1 < entriesInCache1; ++c1)
            {
                OptimizeVertexData<IndexType>& vertexData = vertexDataList[cache1[c1]];
                vertexData.cachePos0 = vertexData.cachePos1;
                vertexData.cachePos1 = kEvictedCacheIndex;
                for (uint32_t j=0; j<vertexData.activeFaceListSize; ++j)
                {
                    uint32_t face = activeFaceList[vertexData.activeFaceListStart+j];
                    float faceScore = 0.f;
                    for (uint32_t v=0; v<3; v++)
                    {
                        OptimizeVertexData<IndexType>& faceVertexData = vertexDataList[vertexRemap[face + v]];
                        faceScore += faceVertexData.score;
                    }
                    if (faceScore > bestScore)
                    {
                        bestScore = faceScore;
                        bestFace = face;
                    }
                }
            }

            std::swap(cache0, cache1);
            entriesInCache0 = std::min(entriesInCache1, lruCacheSize);
        }

        delete [] vertexDataList;
        delete [] vertexRemap;
        delete [] activeFaceList;
        delete [] processedFaceList;
        delete [] faceSorted;
        delete [] faceReverseLookup;
    }

} // ForsythSorted
#endif
/******************************************************************************/
#if METHOD==METHOD_FORSYTH2 || METHOD==METHOD_TEST
/*
  Copyright (C) 2008 Martin Storsjo

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
namespace Forsyth2
{
   #define VERTEX_CACHE_SIZE VTX_CACHE_SIZE

   // The size of these data types affect the memory usage
   #if 0 // this improves speed of optimization but reduces precision
      typedef UShort ScoreType;
      typedef Int    ScoreTypeBig;
      #define SCORE_SCALING 7281
   #else
      typedef Flt ScoreType, ScoreTypeBig;
      #define SCORE_SCALING 1
   #endif

   typedef Byte AdjacencyType;
   #define MAX_ADJACENCY 0xFF

   typedef Int   VertexIndexType;
   typedef SByte CachePosType;
   typedef Int   TriangleIndexType;
   typedef Int   ArrayIndexType;

   // The size of the precalculated tables
   #define   CACHE_SCORE_TABLE_SIZE VTX_CACHE_SIZE
   #define VALENCE_SCORE_TABLE_SIZE VTX_CACHE_SIZE
   #if CACHE_SCORE_TABLE_SIZE<VERTEX_CACHE_SIZE
      #error Vertex score table too small
   #endif

   // Precalculated tables
   static ScoreType cachePositionScore[  CACHE_SCORE_TABLE_SIZE];
   static ScoreType valenceScore      [VALENCE_SCORE_TABLE_SIZE];

   #define ISADDED(x)  (triangleAdded[(x) >> 3] &  (1 << (x & 7)))
   #define SETADDED(x) (triangleAdded[(x) >> 3] |= (1 << (x & 7)))

   // Score function constants
   #define CACHE_DECAY_POWER   1.5f
   #define LAST_TRI_SCORE      0.75f
   #define VALENCE_BOOST_SCALE 2.0f
   #define VALENCE_BOOST_POWER 0.5f

   // Precalculate the tables
   void initForsyth()
   {
      for(int i=0; i<CACHE_SCORE_TABLE_SIZE; i++)
      {
         Flt score=0;
         if(i<3)
         {
            // This vertex was used in the last triangle, so it has a fixed score, which ever of the three it's in.
            // Otherwise, you can get very different answers depending on whether you add the triangle 1,2,3 or 3,1,2-which is silly
            score=LAST_TRI_SCORE;
         }else
         {
            // Points for being high in the cache.
            const Flt scaler=1.0f/(VERTEX_CACHE_SIZE-3);
            score=1.0f-(i-3)*scaler;
            score=Pow(score, CACHE_DECAY_POWER);
         }
         cachePositionScore[i]=(ScoreType) (SCORE_SCALING*score);
      }

      for(int i=1; i<VALENCE_SCORE_TABLE_SIZE; i++)
      {
         // Bonus points for having a low number of tris still to use the vert, so we get rid of lone verts quickly
         Flt valenceBoost=Pow(i, -VALENCE_BOOST_POWER);
         Flt score=VALENCE_BOOST_SCALE*valenceBoost;
         valenceScore[i]=(ScoreType) (SCORE_SCALING*score);
      }
   }

   // Calculate the score for a vertex
   ScoreType findVertexScore(int numActiveTris, int cachePosition)
   {
      if(!numActiveTris)return 0; // No triangles need this vertex
      ScoreType score=0;
      if(InRange(cachePosition, cachePositionScore))score =cachePositionScore[cachePosition];
      if(InRange(numActiveTris,       valenceScore))score+=      valenceScore[numActiveTris];
      return score;
   }

   // The main reordering function
   void reorderForsyth(const VertexIndexType *indices, int nTriangles, int nVertices, VertexIndexType *outputIndices)
   {
      AdjacencyType *numActiveTris=new AdjacencyType[nVertices];
      memset(numActiveTris, 0, sizeof(AdjacencyType)*nVertices);

      // First scan over the vertex data, count the total number of occurrences of each vertex
      for(int i=0; i<3*nTriangles; i++)
      {
         if(numActiveTris[indices[i]]==MAX_ADJACENCY)
         {
            // Unsupported mesh, vertex shared by too many triangles
            delete [] numActiveTris;
            CopyN(outputIndices, indices, nTriangles*3);
            return;
         }
         numActiveTris[indices[i]]++;
      }

      // Allocate the rest of the arrays
      ArrayIndexType *offsets=new ArrayIndexType[nVertices];
      ScoreType *lastScore=new ScoreType[nVertices];
      CachePosType *cacheTag=new CachePosType[nVertices];
      Byte *triangleAdded=new Byte[(nTriangles + 7)/8];
      ScoreType *triangleScore=new ScoreType[nTriangles];
      TriangleIndexType *triangleIndices=new TriangleIndexType[3*nTriangles];
      memset(triangleAdded, 0, sizeof(Byte)*((nTriangles + 7)/8));
      memset(triangleScore, 0, sizeof(ScoreType)*nTriangles);
      memset(triangleIndices, 0, sizeof(TriangleIndexType)*3*nTriangles);
      TriangleIndexType *outTriangles=new TriangleIndexType[nTriangles];

      // Count the triangle array offset for each vertex, initialize the rest of the data.
      int sum=0;
      for(int i=0; i<nVertices; i++)
      {
         offsets[i]=sum;
         sum+=numActiveTris[i];
         numActiveTris[i]=0;
         cacheTag[i]=-1;
      }

      // Fill the vertex data structures with indices to the triangles using each vertex
      for(int i=0; i<nTriangles; i++)
      for(int j=0; j<3; j++)
      {
         int v=indices[3*i + j];
         triangleIndices[offsets[v] + numActiveTris[v]]=i;
         numActiveTris[v]++;
      }

      // Initialize the score for all vertices
      for(int i=0; i<nVertices; i++)
      {
         int        tris=numActiveTris[i],
                  offset=offsets      [i];
         ScoreType score=lastScore    [i]=findVertexScore(tris, cacheTag[i]);
         REPD(j, tris)triangleScore[triangleIndices[offset+j]]+=score;
      }

      // Find the best triangle
      int          bestTriangle=-1;
      ScoreTypeBig bestScore=-1;
      for(int i=0; i<nTriangles; i++)if(triangleScore[i]>bestScore)
      {
         bestScore=triangleScore[i];
         bestTriangle=i;
      }

      // Initialize the cache
      int cache[VERTEX_CACHE_SIZE + 3];
      for(int i=0; i<VERTEX_CACHE_SIZE + 3; i++)cache[i]=-1;

      // Output the currently best triangle, as long as there are triangles left to output
      int outPos=0, scanPos=0;
      while(bestTriangle>=0)
      {
         // Mark the triangle as added
         SETADDED(bestTriangle);
         // Output this triangle
         outTriangles[outPos++]=bestTriangle;
         for(int i=0; i<3; i++)
         {
            // Update this vertex
            int v=indices[3*bestTriangle + i];

            // Check the current cache position, if it is in the cache
            int endpos=cacheTag[v];
            if(endpos<0)endpos=VERTEX_CACHE_SIZE + i;
            // Move all cache entries from the previous position in the cache to the new target position (i) one step backwards
            for(int j=endpos; j>i; j--)
            {
               cache[j]=cache[j-1];
               // If this cache slot contains a real vertex, update its cache tag
               if(cache[j]>=0)cacheTag[cache[j]]++;
            }
            // Insert the current vertex into its new target slot
            cache[i]=v;
            cacheTag[v]=i;

            // Find the current triangle in the list of active triangles and remove it (moving the last triangle in the list to the slot of this triangle).
            for(int j=0; j<numActiveTris[v]; j++)
            {
               if(triangleIndices[offsets[v] + j]==bestTriangle)
               {
                  triangleIndices[offsets[v] + j]=triangleIndices[offsets[v] + numActiveTris[v]-1];
                  break;
               }
            }
            // Shorten the list
            numActiveTris[v]--;
         }
         // Update the scores of all triangles in the cache
         for(int i=0; i<VERTEX_CACHE_SIZE + 3; i++)
         {
            int v=cache[i];
            if(v<0)break;
            if(i>=VERTEX_CACHE_SIZE) // This vertex was pushed outside of the actual cache
            {
               cacheTag[v]=-1;
               cache   [i]=-1;
            }
            ScoreType newScore=findVertexScore(numActiveTris[v], cacheTag[v]);
            ScoreType diff=newScore-lastScore[v];
            for(int j=0; j<numActiveTris[v]; j++)triangleScore[triangleIndices[offsets[v] + j]]+=diff;
            lastScore[v]=newScore;
         }
         // Find the best triangle referenced by vertices in the cache
         bestTriangle=-1;
         bestScore=-1;
         for(int i=0; i<VERTEX_CACHE_SIZE; i++)
         {
            if(cache[i]<0)break;
            int v=cache[i];
            for(int j=0; j<numActiveTris[v]; j++)
            {
               int t=triangleIndices[offsets[v] + j];
               if(triangleScore[t]>bestScore)
               {
                  bestTriangle=t;
                  bestScore=triangleScore[t];
               }
            }
         }
         // If no active triangle was found at all, continue scanning the whole list of triangles
         if(bestTriangle<0)
            for(; scanPos<nTriangles; scanPos++)if(!ISADDED(scanPos)){bestTriangle=scanPos; break;}
      }

      // Convert the triangle index array into a full triangle list
      outPos=0;
      for(int i=0; i<nTriangles; i++)
      {
         int t=outTriangles[i]*3;
         for(int j=0; j<3; j++)outputIndices[outPos++]=indices[t+j];
      }

      // Clean up
      delete [] triangleIndices;
      delete [] offsets;
      delete [] lastScore;
      delete [] numActiveTris;
      delete [] cacheTag;
      delete [] triangleAdded;
      delete [] triangleScore;
      delete [] outTriangles;
   }
} // Forsyth2
#endif
/******************************************************************************/
#if METHOD==METHOD_TIPSIFY || METHOD==METHOD_TEST
namespace Tipsify
{
   #define DEAD_END_STACK_SIZE 128
   #define DEAD_END_STACK_MASK (DEAD_END_STACK_SIZE-1)

   // The size of these data types control the memory usage
   typedef Byte AdjacencyType;
   #define MAX_ADJACENCY 0xFF

   typedef Int VertexIndexType;
   typedef Int TriangleIndexType;
   typedef Int ArrayIndexType;

   #define ISEMITTED(x)  (emitted[(x) >> 3] &  (1 << (x & 7)))
   #define SETEMITTED(x) (emitted[(x) >> 3] |= (1 << (x & 7)))

   // Find the next non-local vertex to continue from
   int skipDeadEnd(const AdjacencyType *liveTriangles,
                   const VertexIndexType *deadEndStack,
                   int& deadEndStackPos,
                   int& deadEndStackStart,
                   int nVertices,
                   int& i) {

      // Next in dead-end stack
      while ((deadEndStackPos & DEAD_END_STACK_MASK) != deadEndStackStart) {
         int d=deadEndStack[(--deadEndStackPos) & DEAD_END_STACK_MASK];
         // Check for live triangles
         if(liveTriangles[d]>0)
            return d;
      }
      // Next in input order
      while (i + 1<nVertices) {
         // Cursor sweeps list only once
         i++;
         // Check for live triangles
         if(liveTriangles[i]>0)
            return i;
      }
      // We are done!
      return -1;
   }

   // Find the next vertex to continue from
   int getNextVertex(int nVertices,
                     int& i,
                     int k,
                     const VertexIndexType *nextCandidates,
                     int numNextCandidates,
                     const ArrayIndexType *cacheTime,
                     int s,
                     const AdjacencyType *liveTriangles,
                     const VertexIndexType *deadEndStack,
                     int& deadEndStackPos,
                     int& deadEndStackStart) {

      // Best candidate
      int n=-1;
      // and priority
      int m=-1;
      for(int j=0; j<numNextCandidates; j++) {
         int v=nextCandidates[j];
         // Must have live triangles
         if(liveTriangles[v]>0) {
            // Initial priority
            int p=0;
            // In cache even after fanning?
            if(s-cacheTime[v] + 2*liveTriangles[v] <= k)
               // Priority is position in cache
               p=s-cacheTime[v];
            // Keep best candidate
            if(p>m) {
               m=p;
               n=v;
            }
         }
      }
      // Reached a dead-end?
      if(n==-1) {
         // Get non-local vertex
         n=skipDeadEnd(liveTriangles, deadEndStack,
                         deadEndStackPos, deadEndStackStart,
                         nVertices, i);
      }
      return n;
   }

   // The main reordering function
   void tipsify(const VertexIndexType *indices,
                            int nTriangles,
                            int nVertices,
                            VertexIndexType *outputIndices,
                            int k=32)
   {
      // Vertex-triangle adjacency

      // Count the occurrances of each vertex
      AdjacencyType *numOccurrances=new AdjacencyType[nVertices];
      memset(numOccurrances, 0, sizeof(AdjacencyType)*nVertices);
      for(int i=0; i<3*nTriangles; i++) {
         int v=indices[i];
         if(numOccurrances[v]==MAX_ADJACENCY) {
            // Unsupported mesh,
            // vertex shared by too many triangles
            delete [] numOccurrances;
            CopyN(outputIndices, indices, nTriangles*3);
            return;
         }
         numOccurrances[v]++;
      }

      // Find the offsets into the adjacency array for each vertex
      int sum=0;
      ArrayIndexType *offsets=new ArrayIndexType[nVertices+1];
      int maxAdjacency=0;
      for(int i=0; i<nVertices; i++) {
         offsets[i]=sum;
         sum+=numOccurrances[i];
         if(numOccurrances[i]>maxAdjacency)
            maxAdjacency=numOccurrances[i];
         numOccurrances[i]=0;
      }
      offsets[nVertices]=sum;

      // Add the triangle indices to the vertices it refers to
      TriangleIndexType *adjacency=new TriangleIndexType[3*nTriangles];
      for(int i=0; i<nTriangles; i++) {
         const VertexIndexType *vptr=&indices[3*i];
         adjacency[offsets[vptr[0]] + numOccurrances[vptr[0]]]=i;
         numOccurrances[vptr[0]]++;
         adjacency[offsets[vptr[1]] + numOccurrances[vptr[1]]]=i;
         numOccurrances[vptr[1]]++;
         adjacency[offsets[vptr[2]] + numOccurrances[vptr[2]]]=i;
         numOccurrances[vptr[2]]++;
      }

      // Per-vertex live triangle counts
      AdjacencyType *liveTriangles=numOccurrances;

      // Per-vertex caching time stamps
      ArrayIndexType *cacheTime=new ArrayIndexType[nVertices];
      memset(cacheTime, 0, sizeof(ArrayIndexType)*nVertices);

      // Dead-end vertex stack
      VertexIndexType *deadEndStack=new VertexIndexType[DEAD_END_STACK_SIZE];
      memset(deadEndStack, 0, sizeof(VertexIndexType)*DEAD_END_STACK_SIZE);
      int deadEndStackPos=0;
      int deadEndStackStart=0;

      // Per triangle emitted flag
      Byte *emitted=new Byte[(nTriangles + 7)/8];
      memset(emitted, 0, sizeof(Byte)*((nTriangles + 7)/8));

      // Empty output buffer
      TriangleIndexType *outputTriangles=new TriangleIndexType[nTriangles];
      int outputPos=0;

      // Arbitrary starting vertex
      int f=0;
      // Time stamp and cursor
      int s=k + 1;
      int i=0;

      VertexIndexType *nextCandidates=new VertexIndexType[3*maxAdjacency];

      // For all valid fanning vertices
      while (f>=0) {
         // 1-ring of next candidates
         int numNextCandidates=0;
         int startOffset=offsets[f];
         int endOffset=offsets[f+1];
         for(int offset=startOffset; offset<endOffset; offset++) {
            int t=adjacency[offset];
            if(!ISEMITTED(t)) {
               const VertexIndexType *vptr=&indices[3*t];
               // Output triangle
               outputTriangles[outputPos++]=t;
               for(int j=0; j<3; j++) {
                  int v=vptr[j];
                  // Add to dead-end stack
                  deadEndStack[(deadEndStackPos++) & DEAD_END_STACK_MASK]=v;
                  if((deadEndStackPos & DEAD_END_STACK_MASK) ==
                      (deadEndStackStart & DEAD_END_STACK_MASK))
                     deadEndStackStart=(deadEndStackStart + 1) & DEAD_END_STACK_MASK;
                  // Register as candidate
                  nextCandidates[numNextCandidates++]=v;
                  // Decrease live triangle count
                  liveTriangles[v]--;
                  // If not in cache
                  if(s-cacheTime[v]>k) {
                     // Set time stamp
                     cacheTime[v]=s;
                     // Increment time stamp
                     s++;
                  }
               }
               // Flag triangle as emitted
               SETEMITTED(t);
            }
         }
         // Select next fanning vertex
         f=getNextVertex(nVertices, i, k, nextCandidates,
                           numNextCandidates, cacheTime, s,
                           liveTriangles, deadEndStack,
                           deadEndStackPos, deadEndStackStart);
      }

      // Clean up
      delete [] nextCandidates;
      delete [] emitted;
      delete [] deadEndStack;
      delete [] cacheTime;
      delete [] adjacency;
      delete [] offsets;
      delete [] numOccurrances;

      // Convert the triangle index array into a full triangle list
      outputPos=0;
      for(int i=0; i<nTriangles; i++) {
         int t=outputTriangles[i];
         for(int j=0; j<3; j++) {
            int v=indices[3*t + j];
            outputIndices[outputPos++]=v;
         }
      }
      delete [] outputTriangles;
   }
} // Tipsify
#endif
/******************************************************************************/
namespace EE{
/******************************************************************************/
MeshBase& MeshBase::optimizeCache(Bool faces, Bool vertexes)
{
   Memt<Int> remap;

   // face reorder
   if(faces)
   {
      quadToTri().exclude(VTX_DUP|ADJ_ALL);

   #if METHOD==METHOD_DIRECTX
      setAdjacencies(true, false); ASSERT(Int(UNUSED32)==-1); // 'setAdjacencies' sets -1 for invalid while these codes operate on UNUSED32
      ASSERT(SIZE(tri.ind    (0))==SIZE(uint32_t)*3);
      ASSERT(SIZE(tri.adjFace(0))==SIZE(uint32_t)*3);
      remap.reserve(Max(tris(), vertexes ? vtxs() : 0)); ASSERT(SIZE(Int)==SIZE(uint32_t));
      remap.setNum(tris());
      if(DirectX::OptimizeFaces((uint32_t*)tri.ind(), tris(), (uint32_t*)tri.adjFace(), (uint32_t*)remap.data(), VTX_CACHE_SIZE, VTX_CACHE_SIZE))
      {
         MeshBase temp(0, 0, remap.elms(), 0, TRI_IND);
         REPA(remap)temp.tri.ind(i)=tri.ind(remap[i]);
         Swap(temp.tri, tri);
      }
   #else
      MeshBase temp(0, 0, tris(), 0, TRI_IND);
   #if   METHOD==METHOD_FORSYTH
      Forsyth::OptimizeFaces(tri.ind()->c, tris()*3, vtxs(), temp.tri.ind()->c, VTX_CACHE_SIZE);
   #elif METHOD==METHOD_FORSYTH_SORTED
      ForsythSorted::OptimizeFaces<Int>(tri.ind()->c, tris()*3, temp.tri.ind()->c, VTX_CACHE_SIZE);
   #elif METHOD==METHOD_FORSYTH2
      Forsyth2::reorderForsyth(tri.ind()->c, tris(), vtxs(), temp.tri.ind()->c);
   #elif METHOD==METHOD_TIPSIFY
      Tipsify::tipsify(tri.ind()->c, tris(), vtxs(), temp.tri.ind()->c);
   #endif
      Swap(temp.tri, tri);
   #endif
   }

   // vtx reorder
   if(vertexes)
   {
      remap.setNum(vtxs());
      SetMemN(remap.data(), 0xFF, remap.elms());
      Int used=0,
     *p= tri.ind()->c; REP( tris()*3){Int v=*p++; if(!InRange(v, remap))return T; if(remap[v]<0)remap[v]=used++;}
      p=quad.ind()->c; REP(quads()*4){Int v=*p++; if(!InRange(v, remap))return T; if(remap[v]<0)remap[v]=used++;}
         vertexes=false; FREP(vtxs()){Int &v=remap[i]; if(v<0)v=used++; if(v!=i)vertexes=true;}
      if(vertexes)
      {
         MeshBase temp(remap.elms(), 0, 0, 0, flag()&VTX_ALL);
         REPA(temp.vtx)copyVtx(i, temp, remap[i]);
         Swap(temp.vtx, vtx);
         REPA(tri )tri .ind(i).remap(remap.data());
         REPA(quad)quad.ind(i).remap(remap.data());
      }
   }
   return T;
}
/******************************************************************************/
MeshRender& MeshRender::optimize(Bool faces, Bool vertexes)
{
   Memt<Int> remap;
   Ptr ind=null;

   // face reorder
#if METHOD!=METHOD_TEST
   if(faces)
#endif
   {
   #if METHOD==METHOD_DIRECTX
      MeshBase temp; if(temp.createInd(_ib))
      {
         temp.vtx._elms=vtxs(); // this is needed for 'setAdjacencies'
         temp.setAdjacencies(true, false);
         remap.reserve(Max(tris(), vertexes ? vtxs() : 0));
         remap.setNum(tris());
         Bool ok=false;
         if(_bone_split)
         {
            Int processed_tris=0; Memt<uint32_t> attributes; attributes.setNum(tris());
            FREP(_bone_splits)
            {
               BoneSplit &bs=_bone_split[i];
               REPD(t, bs.tris)attributes[processed_tris++]=i;
            }
            ok=DirectX::OptimizeFacesEx((uint32_t*)temp.tri.ind(), tris(), (uint32_t*)temp.tri.adjFace(), attributes.data(), (uint32_t*)remap.data(), VTX_CACHE_SIZE, VTX_CACHE_SIZE);
         }else
         {
            ok=DirectX::OptimizeFaces((uint32_t*)temp.tri.ind(), tris(), (uint32_t*)temp.tri.adjFace(), (uint32_t*)remap.data(), VTX_CACHE_SIZE, VTX_CACHE_SIZE);
         }
         if(ok)if(ind=indLock(LOCK_WRITE))
         {
            if(indBit16()){VecUS *tri=(VecUS*)ind; REP(tris())tri[i]=temp.tri.ind(remap[i]);}
            else          {VecI  *tri=(VecI *)ind; REP(tris())tri[i]=temp.tri.ind(remap[i]);}
         }
      }
   #else
      if(ind=indLock())
      {
         Memt<VecI> temp; temp.setNum(tris()*2);
         VecI *org=temp.data(), *reordered=temp.data()+tris();
         if(indBit16())Copy16To32(org, ind, tris()*3);
         else          Copy32To32(org, ind, tris()*3);

         if(_bone_split)
         {
            Int processed_tris=0;
            FREP(_bone_splits)
            {
               BoneSplit &bs=_bone_split[i];
            #if   METHOD==METHOD_FORSYTH
               Forsyth::OptimizeFaces((org+processed_tris)->c, bs.tris*3, vtxs(), (reordered+processed_tris)->c, VTX_CACHE_SIZE);
            #elif METHOD==METHOD_FORSYTH_SORTED
               ForsythSorted::OptimizeFaces<Int>((org+processed_tris)->c, bs.tris*3, (reordered+processed_tris)->c, VTX_CACHE_SIZE);
            #elif METHOD==METHOD_FORSYTH2
               Forsyth2::reorderForsyth((org+processed_tris)->c, bs.tris, vtxs(), (reordered+processed_tris)->c);
            #elif METHOD==METHOD_TIPSIFY
               Tipsify::tipsify((org+processed_tris)->c, bs.tris, vtxs(), (reordered+processed_tris)->c);
            #endif
               processed_tris+=bs.tris;
            }
         }else
         {
         #if   METHOD==METHOD_FORSYTH
            Forsyth::OptimizeFaces(org->c, tris()*3, vtxs(), reordered->c, VTX_CACHE_SIZE);
         #elif METHOD==METHOD_FORSYTH_SORTED
            ForsythSorted::OptimizeFaces<Int>(org->c, tris()*3, reordered->c, VTX_CACHE_SIZE);
         #elif METHOD==METHOD_FORSYTH2
            Forsyth2::reorderForsyth(org->c, tris(), vtxs(), reordered->c);
         #elif METHOD==METHOD_TIPSIFY
            Tipsify::tipsify(org->c, tris(), vtxs(), reordered->c);
         #elif METHOD==METHOD_TEST
            //if(faces)
               Forsyth::OptimizeFaces(org->c, tris()*3, vtxs(), reordered->c, VTX_CACHE_SIZE);
            //else
               //ForsythSorted::OptimizeFaces<Int>(org->c, tris()*3, reordered->c, VTX_CACHE_SIZE);
               //Forsyth2::reorderForsyth(org->c, tris(), vtxs(), reordered->c);
               //Tipsify::tipsify(org->c, tris(), vtxs(), reordered->c);
         #endif
         }

         if(indBit16())Copy32To16(ind, reordered, tris()*3);
         else          Copy32To32(ind, reordered, tris()*3);
      }
   #endif
   }

   // vtx reorder
   if(vertexes)
   {
      if(!ind)ind=indLock();
      if( ind)
      {
         remap.setNum(vtxs());
         SetMemN(remap.data(), 0xFF, remap.elms());
         Int used=0; // iterate all triangle indexes in order, and set new indexes for the vertexes, so they are listed in order as they are used by the triangles
         if(indBit16()){U16 *p=(U16*)ind; REP(tris()*3){Int v=*p++; if(!InRange(v, remap)){vertexes=false; break;} if(remap[v]<0)remap[v]=used++;}}
         else          {U32 *p=(U32*)ind; REP(tris()*3){Int v=*p++; if(!InRange(v, remap)){vertexes=false; break;} if(remap[v]<0)remap[v]=used++;}}
         if(vertexes)
         {
               vertexes=false; FREP(vtxs()){Int &v=remap[i]; if(v<0)v=used++; if(v!=i)vertexes=true;}
            if(vertexes)if(Byte *vtx=vtxLock())
            {
               Memt<Byte> vtx_temp; vtx_temp.setNum(vtxs()*vtxSize());
               REP(vtxs())CopyFast(vtx_temp.data() + remap[i]*vtxSize(), vtx + i*vtxSize(), vtxSize());
               CopyFast(vtx, vtx_temp.data(), vtx_temp.elms());
               vtxUnlock();

               if(indBit16()){VecUS *tri=(VecUS*)ind; REP(tris())tri[i].remap(remap.data());}
               else          {VecI  *tri=(VecI *)ind; REP(tris())tri[i].remap(remap.data());}
            }
         }
      }
   }

   if(ind)indUnlock();
   return T;
}
/******************************************************************************/
void InitMesh()
{
#if METHOD==METHOD_FORSYTH || METHOD==METHOD_TEST
   Forsyth::ComputeVertexScores();
#endif
#if METHOD==METHOD_FORSYTH_SORTED || METHOD==METHOD_TEST
   ForsythSorted::ComputeVertexScores();
#endif
#if METHOD==METHOD_FORSYTH2 || METHOD==METHOD_TEST
   Forsyth2::initForsyth();
#endif
}
/******************************************************************************/
}
/******************************************************************************/
