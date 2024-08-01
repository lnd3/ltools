#include "testing/Test.h"
#include "logging/Log.h"
#include "logging/String.h"

// Copyright (c) 2020 Cesanta Software Limited
// All rights reserved

#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <filesystem>

#include "filesystem/File.h"

#include "mongoose/mongoose.h"
#include "curl/curl.h"

std::thread OnceTimer(std::chrono::seconds time, std::function<void()> timesup) {
    return std::thread([=]() {
        std::this_thread::sleep_for(time);
        timesup();
    });
}

static void MongooseHttpServerCallback(struct mg_connection* c, int ev, void* ev_data, void* fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        if (mg_http_match_uri(hm, "/api/test")) 
            mg_http_reply(c, 200, "", "{12345}");
    }
}

static size_t CurlClientWriteCallback(char* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
};


TEST(Network, MongooseHttpServerTest) {
    std::atomic_bool run = true;

    auto server = std::thread([&]() {
        struct mg_mgr mgr;
        mg_mgr_init(&mgr);                                      // Init manager
        auto conn = mg_http_listen(&mgr, "http://localhost:8088", MongooseHttpServerCallback, &mgr);  // Setup listener


        for (; run;) mg_mgr_poll(&mgr, 1);                       // Event loop
        mg_mgr_free(&mgr);                                      // Cleanup

        });

    {
        CURL* curl;
        CURLcode res;

        /* In windows, this will init the winsock stuff */
        curl_global_init(CURL_GLOBAL_ALL);

        /* get a curl handle */
        curl = curl_easy_init();
        if (curl) {
            /* First set the URL that is about to receive our POST. This URL can
               just as well be an https:// URL if that is what should receive the
               data. */
            curl_easy_setopt(curl, CURLOPT_URL, "http://localhost/api/test");
            curl_easy_setopt(curl, CURLOPT_PORT, 8088);

            std::string readBuffer;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlClientWriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            /* Perform the request, res will get the return code */
            res = curl_easy_perform(curl);
            /* Check for errors */

            if (res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

            TEST_TRUE(res == CURLE_OK, "Failed curl post: " + std::to_string(res));
            TEST_TRUE(readBuffer == "{12345}", "");

            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    run = false;
    server.join();

    return 0;
}



