#ifndef CYBERSENTINEL_HTTP_CLIENT_H
#define CYBERSENTINEL_HTTP_CLIENT_H

#include <string>
#include <memory>

namespace cybersentinel {

struct HttpResponse {
    int status_code;
    std::string body;

    HttpResponse() : status_code(0) {}
    HttpResponse(int code, const std::string& response_body)
        : status_code(code), body(response_body) {}
};

class HttpClient {
public:
    explicit HttpClient(const std::string& base_url);
    ~HttpClient();

    // HTTP methods
    HttpResponse get(const std::string& endpoint);
    HttpResponse post(const std::string& endpoint, const std::string& data);
    HttpResponse put(const std::string& endpoint, const std::string& data);
    HttpResponse del(const std::string& endpoint);

    // Configuration
    void set_timeout(int seconds);
    void set_header(const std::string& key, const std::string& value);

private:
    std::string base_url_;
    int timeout_;

    HttpResponse perform_request(const std::string& method,
                                 const std::string& endpoint,
                                 const std::string& data = "");

    std::string build_url(const std::string& endpoint);
};

} // namespace cybersentinel

#endif // CYBERSENTINEL_HTTP_CLIENT_H
