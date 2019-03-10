// metrohash64.cpp
//
// The MIT License (MIT)
//
// Copyright (c) 2015 J. Andrew Rogers
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "platform.h"
#include "metrohash64.h"

INLINE void MetroHash64::Initialize(const uint64_t seed)
{
    vseed = (seed + k2) * k0;

    // initialize internal hash registers
    state.v[0] = vseed;
    state.v[1] = vseed;
    state.v[2] = vseed;
    state.v[3] = vseed;

    // initialize total length of input
    bytes = 0;
}

INLINE void MetroHash64::Update(const uint8_t *data, const uint64_t length) // ESENTHEL CHANGED
{
   uint64_t input_size=(bytes&31);
   bytes+=length;

   const uint64_t copy=32-input_size;
   if(length<copy){memcpy(input.b+input_size, data, length); return;} // incomplete input buffer

   const uint8_t *const end=data+length;
   State state=this->state;

   if(input_size) // complete input buffer
   {
      // process full input buffer
      memcpy(input.b+input_size, data, copy); data+=copy;
      state.v[0] += read_u64(&input.b[ 0]) * k0; state.v[0] = rotate_right(state.v[0],29) + state.v[2];
      state.v[1] += read_u64(&input.b[ 8]) * k1; state.v[1] = rotate_right(state.v[1],29) + state.v[3];
      state.v[2] += read_u64(&input.b[16]) * k2; state.v[2] = rotate_right(state.v[2],29) + state.v[0];
      state.v[3] += read_u64(&input.b[24]) * k3; state.v[3] = rotate_right(state.v[3],29) + state.v[1];
   }
    
   const uint8_t *const limit=end-32;
   while(data<=limit)
   {
      state.v[0] += read_u64(data) * k0; data += 8; state.v[0] = rotate_right(state.v[0],29) + state.v[2];
      state.v[1] += read_u64(data) * k1; data += 8; state.v[1] = rotate_right(state.v[1],29) + state.v[3];
      state.v[2] += read_u64(data) * k2; data += 8; state.v[2] = rotate_right(state.v[2],29) + state.v[0];
      state.v[3] += read_u64(data) * k3; data += 8; state.v[3] = rotate_right(state.v[3],29) + state.v[1];
   }

   this->state=state;
    
   // store remaining bytes in input buffer
   if(data<end)memcpy(input.b, data, end-data);
}


INLINE void MetroHash64::Finalize(uint8_t *const hash)
{
    State state = this->state; // ESENTHEL CHANGED (using these as temp variables works faster)
   
    // finalize bulk loop, if used
    if (bytes >= 32)
    {
        state.v[2] ^= rotate_right(((state.v[0] + state.v[3]) * k0) + state.v[1], 37) * k1;
        state.v[3] ^= rotate_right(((state.v[1] + state.v[2]) * k1) + state.v[0], 37) * k0;
        state.v[0] ^= rotate_right(((state.v[0] + state.v[2]) * k0) + state.v[3], 37) * k1;
        state.v[1] ^= rotate_right(((state.v[1] + state.v[3]) * k1) + state.v[2], 37) * k0;

        state.v[0] = vseed + (state.v[0] ^ state.v[1]);
    }
    
    // process any bytes remaining in the input buffer
    const uint8_t *      data = input.b;
    const uint8_t *const end  = data + (bytes & 31);
    
    if ((end - data) >= 16)
    {
        state.v[1]  = state.v[0] + (read_u64(data) * k2); data += 8; state.v[1] = rotate_right(state.v[1],29) * k3;
        state.v[2]  = state.v[0] + (read_u64(data) * k2); data += 8; state.v[2] = rotate_right(state.v[2],29) * k3;
        state.v[1] ^= rotate_right(state.v[1] * k0, 21) + state.v[2];
        state.v[2] ^= rotate_right(state.v[2] * k3, 21) + state.v[1];
        state.v[0] += state.v[2];
    }

    if ((end - data) >= 8)
    {
        state.v[0] += read_u64(data) * k3; data += 8;
        state.v[0] ^= rotate_right(state.v[0], 55) * k1;
    }

    if ((end - data) >= 4)
    {
        state.v[0] += read_u32(data) * k3; data += 4;
        state.v[0] ^= rotate_right(state.v[0], 26) * k1;
    }

    if ((end - data) >= 2)
    {
        state.v[0] += read_u16(data) * k3; data += 2;
        state.v[0] ^= rotate_right(state.v[0], 48) * k1;
    }

    if ((end - data) >= 1)
    {
        state.v[0] += read_u8 (data) * k3;
        state.v[0] ^= rotate_right(state.v[0], 37) * k1;
    }
    
    state.v[0] ^= rotate_right(state.v[0], 28);
    state.v[0] *= k0;
    state.v[0] ^= rotate_right(state.v[0], 29);

    bytes = 0;

    // do any endian conversion here

    memcpy(hash, state.v, 8);
}

INLINE void MetroHash64::Hash(const uint8_t *data, const uint64_t length, uint8_t *const hash, const uint64_t seed)
{
    const uint8_t *const end=data+length;

    uint64_t h = (seed + k2) * k0;

    if (length >= 32)
    {
        uint64_t v[4];
        v[0] = h;
        v[1] = h;
        v[2] = h;
        v[3] = h;

        do
        {
            v[0] += read_u64(data) * k0; data += 8; v[0] = rotate_right(v[0],29) + v[2];
            v[1] += read_u64(data) * k1; data += 8; v[1] = rotate_right(v[1],29) + v[3];
            v[2] += read_u64(data) * k2; data += 8; v[2] = rotate_right(v[2],29) + v[0];
            v[3] += read_u64(data) * k3; data += 8; v[3] = rotate_right(v[3],29) + v[1];
        }
        while (data <= (end - 32));

        v[2] ^= rotate_right(((v[0] + v[3]) * k0) + v[1], 37) * k1;
        v[3] ^= rotate_right(((v[1] + v[2]) * k1) + v[0], 37) * k0;
        v[0] ^= rotate_right(((v[0] + v[2]) * k0) + v[3], 37) * k1;
        v[1] ^= rotate_right(((v[1] + v[3]) * k1) + v[2], 37) * k0;
        h += v[0] ^ v[1];
    }

    if ((end - data) >= 16)
    {
        uint64_t v0 = h + (read_u64(data) * k2); data += 8; v0 = rotate_right(v0,29) * k3;
        uint64_t v1 = h + (read_u64(data) * k2); data += 8; v1 = rotate_right(v1,29) * k3;
        v0 ^= rotate_right(v0 * k0, 21) + v1;
        v1 ^= rotate_right(v1 * k3, 21) + v0;
        h += v1;
    }

    if ((end - data) >= 8)
    {
        h += read_u64(data) * k3; data += 8;
        h ^= rotate_right(h, 55) * k1;
    }

    if ((end - data) >= 4)
    {
        h += read_u32(data) * k3; data += 4;
        h ^= rotate_right(h, 26) * k1;
    }

    if ((end - data) >= 2)
    {
        h += read_u16(data) * k3; data += 2;
        h ^= rotate_right(h, 48) * k1;
    }

    if ((end - data) >= 1)
    {
        h += read_u8 (data) * k3;
        h ^= rotate_right(h, 37) * k1;
    }

    h ^= rotate_right(h, 28);
    h *= k0;
    h ^= rotate_right(h, 29);

    memcpy(hash, &h, 8);
}

const char * MetroHash64::test_string = "012345678901234567890123456789012345678901234567890123456789012";

const uint8_t MetroHash64::test_seed_0[8] =   { 0x6B, 0x75, 0x3D, 0xAE, 0x06, 0x70, 0x4B, 0xAD };
const uint8_t MetroHash64::test_seed_1[8] =   { 0x3B, 0x0D, 0x48, 0x1C, 0xF4, 0xB9, 0xB8, 0xDF };

bool MetroHash64::ImplementationVerified()
{
    uint8_t hash[8];
    const uint8_t *key = reinterpret_cast<const uint8_t *>(MetroHash64::test_string);

    // verify one-shot implementation
    MetroHash64::Hash(key, strlen(MetroHash64::test_string), hash, 0);
    if (memcmp(hash, MetroHash64::test_seed_0, 8) != 0) return false;

    MetroHash64::Hash(key, strlen(MetroHash64::test_string), hash, 1);
    if (memcmp(hash, MetroHash64::test_seed_1, 8) != 0) return false;

    // verify incremental implementation
    MetroHash64 metro;
    
    metro.Initialize(0);
    metro.Update(reinterpret_cast<const uint8_t *>(MetroHash64::test_string), strlen(MetroHash64::test_string));
    metro.Finalize(hash);
    if (memcmp(hash, MetroHash64::test_seed_0, 8) != 0) return false;

    metro.Initialize(1);
    metro.Update(reinterpret_cast<const uint8_t *>(MetroHash64::test_string), strlen(MetroHash64::test_string));
    metro.Finalize(hash);
    if (memcmp(hash, MetroHash64::test_seed_1, 8) != 0) return false;

    return true;
}
