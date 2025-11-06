#include "http_client.h"
#include "logger.h"
#include <curl/curl.h>
#include <sstream>

namespace cybersentinel {

// Callback for curl to write response data
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

HttpClient::HttpClient(const std::string& base_url)
    : base_url_(base_url), timeout_(30) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpClient::~HttpClient() {
    curl_global_cleanup();
}

void HttpClient::set_timeout(int seconds) {
    timeout_ = seconds;
}

void HttpClient::set_header(const std::string& key, const std::string& value) {
    // Headers can be added per request if needed
}

HttpResponse HttpClient::get(const std::string& endpoint) {
    return perform_request("GET", endpoint);
}

HttpResponse HttpClient::post(const std::string& endpoint, const std::string& data) {
    return perform_request("POST", endpoint, data);
}

HttpResponse HttpClient::put(const std::string& endpoint, const std::string& data) {
    return perform_request("PUT", endpoint, data);
}

HttpResponse HttpClient::del(const std::string& endpoint) {
    return perform_request("DELETE", endpoint);
}

std::string HttpClient::build_url(const std::string& endpoint) {
    std::string url = base_url_;
    if (!url.empty() && url.back() == '/' && !endpoint.empty() && endpoint[0] == '/') {
        url.pop_back();
    }
    return url + endpoint;
}

HttpResponse HttpClient::perform_request(const std::string& method,
                                        const std::string& endpoint,
                                        const std::string& data) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        Logger::error("Failed to initialize CURL");
        return HttpResponse(0, "");
    }

    std::string response_body;
    std::string url = build_url(endpoint);
    long response_code = 0;

    try {
        // Set URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set timeout
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_);

        // Set response callback
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

        // Set headers
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set method and data
        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        } else if (method == "PUT") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        } else if (method == "DELETE") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }
        // GET is default

        // Perform request
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            Logger::error("HTTP request failed: " + std::string(curl_easy_strerror(res)));
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return HttpResponse(0, "");
        }

        // Get response code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return HttpResponse(static_cast<int>(response_code), response_body);

    } catch (const std::exception& e) {
        Logger::error("HTTP request exception: " + std::string(e.what()));
        curl_easy_cleanup(curl);
        return HttpResponse(0, "");
    }
}

} // namespace cybersentinel
