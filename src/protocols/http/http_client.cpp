#include "protocols/http/http_client.h"
#include "utils/logger.h"

namespace iot::protocols::http {

HttpClient::HttpClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpClient::~HttpClient() {
    curl_global_cleanup();
}

size_t HttpClient::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    static_cast<std::string *>(userp)->append(static_cast<char *>(contents), size * nmemb);
    return size * nmemb;
}

HttpResponse HttpClient::get(const std::string& url, const std::map<std::string, std::string>& headers) {
    CURL* curl = curl_easy_init();
    curl_slist* header_list = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }

    HttpResponse response = performRequest(curl, url, header_list);
    
    if (header_list) curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);
    return response;
}

HttpResponse HttpClient::post(const std::string& url, const std::string& payload, const std::map<std::string, std::string>& headers) {
    CURL* curl = curl_easy_init();
    curl_slist* header_list = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    HttpResponse response = performRequest(curl, url, header_list);

    if (header_list) curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);
    return response;
}

HttpResponse HttpClient::put(const std::string& url, const std::string& payload, const std::map<std::string, std::string>& headers) {
    CURL* curl = curl_easy_init();
    curl_slist* header_list = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    HttpResponse response = performRequest(curl, url, header_list);

    if (header_list) curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);
    return response;
}

HttpResponse HttpClient::del(const std::string& url, const std::map<std::string, std::string>& headers) {
    CURL* curl = curl_easy_init();
    curl_slist* header_list = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    HttpResponse response = performRequest(curl, url, header_list);

    if (header_list) curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);
    return response;
}

HttpResponse HttpClient::performRequest(CURL* curl, const std::string& url, curl_slist* header_list) {
    HttpResponse response = {0, "", ""};
    if (!curl) {
        response.error = "Failed to initialize CURL";
        return response;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (header_list) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // 10 seconds timeout

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        response.error = curl_easy_strerror(res);
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
    }

    return response;
}

} // namespace iot::protocols::http
