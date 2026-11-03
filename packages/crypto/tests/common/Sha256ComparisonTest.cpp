#include "testing/Test.h"
#include "logging/Log.h"

#include "crypto/Sha256.h"
#include "cryptopp/hmac.h"
#include "cryptopp/sha.h"
#include "cryptopp/hex.h"

#include <random>

using namespace l;

// Helper to convert CryptoPP's binary digest to hex string
std::string CryptoPPDigestToHex(const CryptoPP::byte* digest, size_t len) {
    std::string result;
    CryptoPP::StringSource(digest, len, true,
        new CryptoPP::HexEncoder(new CryptoPP::StringSink(result)));
    return result;
}

TEST(Sha256Verification, EmptyString) {
    std::string expected = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    l::crypto::SHA256 custom;
    custom.Update("");
    std::string custom_result = custom.Final();
    TEST_TRUE(custom_result == expected, "Custom empty string");

    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256().CalculateDigest(digest, (CryptoPP::byte*)"", 0);
    std::string cryptopp_result = CryptoPPDigestToHex(digest, CryptoPP::SHA256::DIGESTSIZE);
    TEST_TRUE(cryptopp_result == expected, "CryptoPP empty string");
    TEST_TRUE(custom_result == cryptopp_result, "Custom matches CryptoPP");
    return 0;
}

TEST(Sha256Verification, ABC) {
    std::string expected = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";

    l::crypto::SHA256 custom;
    custom.Update("abc");
    std::string custom_result = custom.Final();
    TEST_TRUE(custom_result == expected, "Custom 'abc'");

    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256().CalculateDigest(digest, (CryptoPP::byte*)"abc", 3);
    std::string cryptopp_result = CryptoPPDigestToHex(digest, CryptoPP::SHA256::DIGESTSIZE);
    TEST_TRUE(cryptopp_result == expected, "CryptoPP 'abc'");
    TEST_TRUE(custom_result == cryptopp_result, "Both match");
    return 0;
}

TEST(Sha256Verification, LongString) {
    std::string input = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    std::string expected = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";

    l::crypto::SHA256 custom;
    custom.Update(input);
    std::string custom_result = custom.Final();
    TEST_TRUE(custom_result == expected, "Custom long string");

    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256().CalculateDigest(digest, (CryptoPP::byte*)input.c_str(), input.length());
    std::string cryptopp_result = CryptoPPDigestToHex(digest, CryptoPP::SHA256::DIGESTSIZE);
    TEST_TRUE(cryptopp_result == expected, "CryptoPP long string");
    TEST_TRUE(custom_result == cryptopp_result, "Long string match");
    return 0;
}

TEST(Sha256Verification, HMAC_Vector1) {
    std::string key = "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b";
    std::string data = "Hi There";
    std::string expected = "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7";

    std::string custom_result = l::crypto::HmacSha256::Compute(key, data);
    TEST_TRUE(custom_result == expected, "Custom HMAC-SHA256");

    CryptoPP::HMAC<CryptoPP::SHA256> hmac((CryptoPP::byte*)key.c_str(), key.length());
    hmac.Update((CryptoPP::byte*)data.c_str(), data.length());
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    hmac.Final(digest);
    std::string cryptopp_result = CryptoPPDigestToHex(digest, CryptoPP::SHA256::DIGESTSIZE);
    TEST_TRUE(cryptopp_result == expected, "CryptoPP HMAC-SHA256");
    TEST_TRUE(custom_result == cryptopp_result, "HMAC results match");
    return 0;
}

TEST(Sha256Verification, HMAC_Vector2) {
    std::string key = "Jefe";
    std::string data = "what do ya want for nothing?";
    std::string expected = "5bdcc146bf60754e6a042426089f0659f0ea838fcd0c6b6b0e4474e8f5f37c3b";

    std::string custom_result = l::crypto::HmacSha256::Compute(key, data);
    TEST_TRUE(custom_result == expected, "Custom HMAC test 2");

    CryptoPP::HMAC<CryptoPP::SHA256> hmac((CryptoPP::byte*)key.c_str(), key.length());
    hmac.Update((CryptoPP::byte*)data.c_str(), data.length());
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    hmac.Final(digest);
    std::string cryptopp_result = CryptoPPDigestToHex(digest, CryptoPP::SHA256::DIGESTSIZE);
    TEST_TRUE(cryptopp_result == expected, "CryptoPP HMAC test 2");
    TEST_TRUE(custom_result == cryptopp_result, "Test 2 match");
    return 0;
}

TEST(Sha256Verification, RandomStressData) {
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dis(0, 255);

    int mismatches = 0;
    for (int test_case = 0; test_case < 50; ++test_case) {
        int data_len = dis(gen) % 5000 + 1;
        std::string data;
        data.reserve(data_len);

        for (int i = 0; i < data_len; ++i) {
            data.push_back((char)dis(gen));
        }

        l::crypto::SHA256 custom;
        custom.Update(data);
        std::string custom_result = custom.Final();

        CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
        CryptoPP::SHA256().CalculateDigest(digest, (CryptoPP::byte*)data.c_str(), data.length());
        std::string cryptopp_result = CryptoPPDigestToHex(digest, CryptoPP::SHA256::DIGESTSIZE);

        if (custom_result != cryptopp_result) {
            mismatches++;
        }
    }
    TEST_TRUE(mismatches == 0, "All random data stress tests match");
    return 0;
}

TEST(Sha256Verification, IncrementalUpdates) {
    std::string full_data = "The quick brown fox jumps over the lazy dog";

    l::crypto::SHA256 full_hasher;
    full_hasher.Update(full_data);
    std::string full_result = full_hasher.Final();

    l::crypto::SHA256 incremental_hasher;
    for (size_t i = 0; i < full_data.length(); i += 5) {
        size_t chunk_len = std::min(size_t(5), full_data.length() - i);
        incremental_hasher.Update(full_data.substr(i, chunk_len));
    }
    std::string incremental_result = incremental_hasher.Final();

    TEST_TRUE(full_result == incremental_result, "Incremental and full update match");

    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256().CalculateDigest(digest, (CryptoPP::byte*)full_data.c_str(), full_data.length());
    std::string cryptopp_result = CryptoPPDigestToHex(digest, CryptoPP::SHA256::DIGESTSIZE);

    TEST_TRUE(full_result == cryptopp_result, "Custom matches CryptoPP");
    return 0;
}
