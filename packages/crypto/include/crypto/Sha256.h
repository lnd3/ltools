#pragma once

#include <cstdint>
#include <cstring>
#include <string>

// Lightweight SHA256 implementation (public domain)
// Based on RFC 6234

namespace l::crypto {

class SHA256 {
public:
    SHA256() { Init(); }

    void Init() {
        mH[0] = 0x6a09e667;
        mH[1] = 0xbb67ae85;
        mH[2] = 0x3c6ef372;
        mH[3] = 0xa54ff53a;
        mH[4] = 0x510e527f;
        mH[5] = 0x9b05688c;
        mH[6] = 0x1f83d9ab;
        mH[7] = 0x5be0cd19;
        mLen = 0;
        mMsgIdx = 0;
    }

    void Update(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            mMsg[mMsgIdx++] = data[i];
            if (mMsgIdx == 64) {
                ProcessBlock();
                mMsgIdx = 0;
            }
        }
        mLen += len;
    }

    void Update(const std::string& str) {
        Update((const uint8_t*)str.c_str(), str.length());
    }

    std::string Final() {
        uint8_t digest[32];
        Finalize(digest);

        std::string result;
        for (int i = 0; i < 32; ++i) {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02x", digest[i]);
            result += buf;
        }
        return result;
    }

    // Get raw 32-byte digest (for comparison testing)
    void Finalize(uint8_t* digest) {
        // Create working copy of current message block
        uint8_t work[128];
        memset(work, 0, sizeof(work));
        memcpy(work, mMsg, mMsgIdx);

        // Append the '1' bit (0x80)
        work[mMsgIdx] = 0x80;

        // Append '0' bits until message is 56 mod 64
        size_t idx = mMsgIdx + 1;
        while ((idx % 64) != 56) {
            work[idx] = 0x00;
            idx++;
        }

        // Append original message length in bits as 64-bit big-endian
        uint64_t bitLen = mLen * 8;
        for (int i = 7; i >= 0; --i) {
            work[idx + i] = (uint8_t)(bitLen & 0xff);
            bitLen >>= 8;
        }
        idx += 8;

        // Process final block(s)
        for (size_t i = 0; i < idx; i += 64) {
            memcpy(mMsg, work + i, 64);
            ProcessBlock();
        }

        // Output hash as hex string
        for (int i = 0; i < 8; ++i) {
            digest[i * 4] = (uint8_t)(mH[i] >> 24);
            digest[i * 4 + 1] = (uint8_t)(mH[i] >> 16);
            digest[i * 4 + 2] = (uint8_t)(mH[i] >> 8);
            digest[i * 4 + 3] = (uint8_t)mH[i];
        }
    }

private:
    static constexpr uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
        0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
        0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
        0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
        0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    uint32_t mH[8];
    uint8_t mMsg[64];
    size_t mLen;
    int mMsgIdx;

    static uint32_t RightRotate(uint32_t x, int n) {
        return (x >> n) | (x << (32 - n));
    }

    void ProcessBlock() {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = ((uint32_t)mMsg[i * 4] << 24) | ((uint32_t)mMsg[i * 4 + 1] << 16) |
                   ((uint32_t)mMsg[i * 4 + 2] << 8) | (uint32_t)mMsg[i * 4 + 3];
        }
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = RightRotate(w[i - 15], 7) ^ RightRotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = RightRotate(w[i - 2], 17) ^ RightRotate(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = mH[0], b = mH[1], c = mH[2], d = mH[3];
        uint32_t e = mH[4], f = mH[5], g = mH[6], h = mH[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = RightRotate(e, 6) ^ RightRotate(e, 11) ^ RightRotate(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = h + S1 + ch + K[i] + w[i];
            uint32_t S0 = RightRotate(a, 2) ^ RightRotate(a, 13) ^ RightRotate(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        mH[0] += a;
        mH[1] += b;
        mH[2] += c;
        mH[3] += d;
        mH[4] += e;
        mH[5] += f;
        mH[6] += g;
        mH[7] += h;
    }
};

// HMAC-SHA256 using custom SHA256
class HmacSha256 {
public:
    static std::string Compute(const std::string& key, const std::string& message) {
        const int BLOCK_SIZE = 64;
        const uint8_t IPAD = 0x36;
        const uint8_t OPAD = 0x5c;

        std::string key_padded = key;
        if (key_padded.length() > BLOCK_SIZE) {
            SHA256 sha;
            sha.Update(key_padded);
            key_padded = sha.Final();
        }
        key_padded.resize(BLOCK_SIZE, 0);

        std::string ipad(BLOCK_SIZE, IPAD);
        std::string opad(BLOCK_SIZE, OPAD);

        for (int i = 0; i < BLOCK_SIZE; ++i) {
            ipad[i] ^= key_padded[i];
            opad[i] ^= key_padded[i];
        }

        SHA256 sha_inner;
        sha_inner.Update(ipad);
        sha_inner.Update(message);
        std::string inner_hash = sha_inner.Final();

        SHA256 sha_outer;
        sha_outer.Update(opad);
        sha_outer.Update(inner_hash);
        return sha_outer.Final();
    }
};

}  // namespace l::crypto
