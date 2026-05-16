#include "protocols/http/api_service.h"
#include <utility>

namespace iot::protocols::http {

BaseApiService::BaseApiService(std::string baseUrl) 
    : baseUrl_(std::move(baseUrl)), client_(std::make_unique<HttpClient>()) {}

void BaseApiService::setAuthToken(const std::string& token) {
    authToken_ = token;
}

void BaseApiService::setBaseUrl(const std::string& baseUrl) {
    baseUrl_ = baseUrl;
}

HttpResponse BaseApiService::get(const std::string& endpoint, const std::map<std::string, std::string>& headers) {
    return HttpClient::get(fullUrl(endpoint), prepareHeaders(headers));
}

HttpResponse BaseApiService::post(const std::string& endpoint, const std::string& payload, const std::map<std::string, std::string>& headers) {
    return HttpClient::post(fullUrl(endpoint), payload, prepareHeaders(headers));
}

HttpResponse BaseApiService::put(const std::string& endpoint, const std::string& payload, const std::map<std::string, std::string>& headers) {
    return HttpClient::put(fullUrl(endpoint), payload, prepareHeaders(headers));
}

HttpResponse BaseApiService::del(const std::string& endpoint, const std::map<std::string, std::string>& headers) {
    return HttpClient::del(fullUrl(endpoint), prepareHeaders(headers));
}

std::map<std::string, std::string> BaseApiService::prepareHeaders(const std::map<std::string, std::string>& extraHeaders) const {
    std::map<std::string, std::string> headers = extraHeaders;
    
    // Default headers
    if (headers.find("Content-Type") == headers.end()) {
        headers["Content-Type"] = "application/json";
    }
    
    if (!authToken_.empty()) {
        headers["Authorization"] = "Bearer " + authToken_;
    }
    
    return headers;
}

std::string BaseApiService::fullUrl(const std::string& endpoint) {
    if (endpoint.empty()) return baseUrl_;
    if (baseUrl_.back() == '/' || endpoint.front() == '/') {
        return baseUrl_ + endpoint;
    }
    return baseUrl_ + "/" + endpoint;
}

} // namespace iot::protocols::http
