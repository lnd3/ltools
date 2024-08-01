#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

#include "curl/curl.h"

using namespace l;

TEST(Network, Curl) {
    CURL* curl;
    CURLcode res;

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);
    CURLversion version{};
    auto version_data = curl_version_info(version);
    /* get a curl handle */
    curl = curl_easy_init();
    if (curl) {
        /* First set the URL that is about to receive our POST. This URL can
           just as well be an https:// URL if that is what should receive the
           data. */
        curl_easy_setopt(curl, CURLOPT_URL, "https://httpbin.org/anything");

        auto res_verify_peer = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        TEST_TRUE(res_verify_peer == CURLE_OK, "Failed to verify peer: " + std::to_string(res_verify_peer));
        auto res_verify_host = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        TEST_TRUE(res_verify_host == CURLE_OK, "Failed to verify host: " + std::to_string(res_verify_host));
        //auto res_ssl = curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, (long)CURLSSLOPT_ALLOW_BEAST | CURLSSLOPT_NO_REVOKE);
        //TEST_TRUE(res_ssl == CURLE_OK, "Failed to set ssl options: " + std::to_string(res_ssl));

        /* Now specify the POST data */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */

        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

        TEST_TRUE(res == CURLE_OK, "Failed curl post: " + std::to_string(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

	return 0;
}
