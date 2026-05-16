#pragma once

#include <string>
#include <map>
#include <curl/curl.h>

namespace iot::protocols::http {

    struct HttpResponse {
        long status_code;
        std::string body;
        std::string error;
    };

    class HttpClient {
    public:
        HttpClient();
        ~HttpClient();

        static HttpResponse get(const std::string& url, const std::map<std::string, std::string>& headers = {});
        static HttpResponse post(const std::string& url, const std::string& payload, const std::map<std::string, std::string>& headers = {});
        static HttpResponse put(const std::string& url, const std::string& payload, const std::map<std::string, std::string>& headers = {});
        static HttpResponse del(const std::string& url, const std::map<std::string, std::string>& headers = {});

    private:
        static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
        static HttpResponse performRequest(CURL* curl, const std::string& url, curl_slist* header_list);
    };

} // namespace iot::protocols::http
