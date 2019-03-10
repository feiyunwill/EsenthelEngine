// metrohash128.cpp
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
#include "metrohash128.h"

INLINE void MetroHash128::Initialize(const uint64_t seed)
{
    // initialize internal hash registers
    state.v[0] = (seed - k0) * k3;
    state.v[1] = (seed + k1) * k2;
    state.v[2] = (seed + k0) * k2;
    state.v[3] = (seed - k1) * k3;

    // initialize total length of input
    bytes = 0;
}

INLINE void MetroHash128::Update(const uint8_t *data, const uint64_t length)
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

INLINE void MetroHash128::Finalize(uint8_t *const hash)
{
    State state = this->state; // ESENTHEL CHANGED (using these as temp variables works faster)

    // finalize bulk loop, if used
    if (bytes >= 32)
    {
        state.v[2] ^= rotate_right(((state.v[0] + state.v[3]) * k0) + state.v[1], 21) * k1;
        state.v[3] ^= rotate_right(((state.v[1] + state.v[2]) * k1) + state.v[0], 21) * k0;
        state.v[0] ^= rotate_right(((state.v[0] + state.v[2]) * k0) + state.v[3], 21) * k1;
        state.v[1] ^= rotate_right(((state.v[1] + state.v[3]) * k1) + state.v[2], 21) * k0;
    }
    
    // process any bytes remaining in the input buffer
    const uint8_t * data = input.b;
    const uint8_t * const end = data + (bytes & 31);
    
    if ((end - data) >= 16)
    {
        state.v[0] += read_u64(data) * k2; data += 8; state.v[0] = rotate_right(state.v[0],33) * k3;
        state.v[1] += read_u64(data) * k2; data += 8; state.v[1] = rotate_right(state.v[1],33) * k3;
        state.v[0] ^= rotate_right((state.v[0] * k2) + state.v[1], 45) * k1;
        state.v[1] ^= rotate_right((state.v[1] * k3) + state.v[0], 45) * k0;
    }

    if ((end - data) >= 8)
    {
        state.v[0] += read_u64(data) * k2; data += 8; state.v[0] = rotate_right(state.v[0],33) * k3;
        state.v[0] ^= rotate_right((state.v[0] * k2) + state.v[1], 27) * k1;
    }

    if ((end - data) >= 4)
    {
        state.v[1] += read_u32(data) * k2; data += 4; state.v[1] = rotate_right(state.v[1],33) * k3;
        state.v[1] ^= rotate_right((state.v[1] * k3) + state.v[0], 46) * k0;
    }

    if ((end - data) >= 2)
    {
        state.v[0] += read_u16(data) * k2; data += 2; state.v[0] = rotate_right(state.v[0],33) * k3;
        state.v[0] ^= rotate_right((state.v[0] * k2) + state.v[1], 22) * k1;
    }

    if ((end - data) >= 1)
    {
        state.v[1] += read_u8 (data) * k2; state.v[1] = rotate_right(state.v[1],33) * k3;
        state.v[1] ^= rotate_right((state.v[1] * k3) + state.v[0], 58) * k0;
    }
    
    state.v[0] += rotate_right((state.v[0] * k0) + state.v[1], 13);
    state.v[1] += rotate_right((state.v[1] * k1) + state.v[0], 37);
    state.v[0] += rotate_right((state.v[0] * k2) + state.v[1], 13);
    state.v[1] += rotate_right((state.v[1] * k3) + state.v[0], 37);

    bytes = 0;

    // do any endian conversion here

    memcpy(hash, state.v, 16);
}

INLINE void MetroHash128::Hash(const uint8_t *data, const uint64_t length, uint8_t * const hash, const uint64_t seed)
{
    const uint8_t *const end = data + length;

    uint64_t v[4];

    v[0] = (seed - k0) * k3;
    v[1] = (seed + k1) * k2;

    if (length >= 32)
    {
        v[2] = (seed + k0) * k2;
        v[3] = (seed - k1) * k3;

        do
        {
            v[0] += read_u64(data) * k0; data += 8; v[0] = rotate_right(v[0],29) + v[2];
            v[1] += read_u64(data) * k1; data += 8; v[1] = rotate_right(v[1],29) + v[3];
            v[2] += read_u64(data) * k2; data += 8; v[2] = rotate_right(v[2],29) + v[0];
            v[3] += read_u64(data) * k3; data += 8; v[3] = rotate_right(v[3],29) + v[1];
        }
        while (data <= (end - 32));

        v[2] ^= rotate_right(((v[0] + v[3]) * k0) + v[1], 21) * k1;
        v[3] ^= rotate_right(((v[1] + v[2]) * k1) + v[0], 21) * k0;
        v[0] ^= rotate_right(((v[0] + v[2]) * k0) + v[3], 21) * k1;
        v[1] ^= rotate_right(((v[1] + v[3]) * k1) + v[2], 21) * k0;
    }

    if ((end - data) >= 16)
    {
        v[0] += read_u64(data) * k2; data += 8; v[0] = rotate_right(v[0],33) * k3;
        v[1] += read_u64(data) * k2; data += 8; v[1] = rotate_right(v[1],33) * k3;
        v[0] ^= rotate_right((v[0] * k2) + v[1], 45) * k1;
        v[1] ^= rotate_right((v[1] * k3) + v[0], 45) * k0;
    }

    if ((end - data) >= 8)
    {
        v[0] += read_u64(data) * k2; data += 8; v[0] = rotate_right(v[0],33) * k3;
        v[0] ^= rotate_right((v[0] * k2) + v[1], 27) * k1;
    }

    if ((end - data) >= 4)
    {
        v[1] += read_u32(data) * k2; data += 4; v[1] = rotate_right(v[1],33) * k3;
        v[1] ^= rotate_right((v[1] * k3) + v[0], 46) * k0;
    }

    if ((end - data) >= 2)
    {
        v[0] += read_u16(data) * k2; data += 2; v[0] = rotate_right(v[0],33) * k3;
        v[0] ^= rotate_right((v[0] * k2) + v[1], 22) * k1;
    }

    if ((end - data) >= 1)
    {
        v[1] += read_u8 (data) * k2; v[1] = rotate_right(v[1],33) * k3;
        v[1] ^= rotate_right((v[1] * k3) + v[0], 58) * k0;
    }

    v[0] += rotate_right((v[0] * k0) + v[1], 13);
    v[1] += rotate_right((v[1] * k1) + v[0], 37);
    v[0] += rotate_right((v[0] * k2) + v[1], 13);
    v[1] += rotate_right((v[1] * k3) + v[0], 37);

    // do any endian conversion here

    memcpy(hash, v, 16);
}

const char * MetroHash128::test_string = "012345678901234567890123456789012345678901234567890123456789012";

const uint8_t MetroHash128::test_seed_0[16] =   {
                                                0xC7, 0x7C, 0xE2, 0xBF, 0xA4, 0xED, 0x9F, 0x9B,
                                                0x05, 0x48, 0xB2, 0xAC, 0x50, 0x74, 0xA2, 0x97
                                                };

const uint8_t MetroHash128::test_seed_1[16] =   {
                                                0x45, 0xA3, 0xCD, 0xB8, 0x38, 0x19, 0x9D, 0x7F,
                                                0xBD, 0xD6, 0x8D, 0x86, 0x7A, 0x14, 0xEC, 0xEF
                                                };

bool MetroHash128::ImplementationVerified()
{
    uint8_t hash[16];
    const uint8_t * key = reinterpret_cast<const uint8_t *>(MetroHash128::test_string);

    // verify one-shot implementation
    MetroHash128::Hash(key, strlen(MetroHash128::test_string), hash, 0);
    if (memcmp(hash, MetroHash128::test_seed_0, 16) != 0) return false;

    MetroHash128::Hash(key, strlen(MetroHash128::test_string), hash, 1);
    if (memcmp(hash, MetroHash128::test_seed_1, 16) != 0) return false;

    // verify incremental implementation
    MetroHash128 metro;
    
    metro.Initialize(0);
    metro.Update(reinterpret_cast<const uint8_t *>(MetroHash128::test_string), strlen(MetroHash128::test_string));
    metro.Finalize(hash);
    if (memcmp(hash, MetroHash128::test_seed_0, 16) != 0) return false;

    metro.Initialize(1);
    metro.Update(reinterpret_cast<const uint8_t *>(MetroHash128::test_string), strlen(MetroHash128::test_string));
    metro.Finalize(hash);
    if (memcmp(hash, MetroHash128::test_seed_1, 16) != 0) return false;

    return true;
}
