#pragma once

#include <string>
#include <memory>
#include "protocols/http/http_client.h"

namespace iot::protocols::http {

    class BaseApiService {
    public:
        explicit BaseApiService(std::string baseUrl);
        virtual ~BaseApiService() = default;

        void setAuthToken(const std::string& token);
        void setBaseUrl(const std::string& baseUrl);

    protected:
        HttpResponse get(const std::string& endpoint, const std::map<std::string, std::string>& headers = {});
        HttpResponse post(const std::string& endpoint, const std::string& payload, const std::map<std::string, std::string>& headers = {});
        HttpResponse put(const std::string& endpoint, const std::string& payload, const std::map<std::string, std::string>& headers = {});
        HttpResponse del(const std::string& endpoint, const std::map<std::string, std::string>& headers = {});

    private:
        std::string baseUrl_;
        std::string authToken_;
        std::unique_ptr<HttpClient> client_;

        [[nodiscard]] std::map<std::string, std::string> prepareHeaders(const std::map<std::string, std::string>& extraHeaders) const;
        std::string fullUrl(const std::string& endpoint);
    };

} // namespace iot::protocols::http
