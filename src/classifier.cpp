#include "classifier.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace cybersentinel {

Classifier::Classifier() {
    // Initialize regex patterns

    // Credit card (PAN) - matches various formats
    pan_regex_ = std::regex(R"(\b\d{4}[-\s]?\d{4}[-\s]?\d{4}[-\s]?\d{4}\b)");

    // Social Security Number
    ssn_regex_ = std::regex(R"(\b\d{3}-\d{2}-\d{4}\b)");

    // Email address
    email_regex_ = std::regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");

    // API keys (common patterns)
    api_key_regex_ = std::regex(R"((api[_-]?key|apikey|access[_-]?token|secret[_-]?key)[:\s=]+['\"]?([a-zA-Z0-9_-]{20,})['\"]?)", std::regex::icase);

    // Generic secrets
    secret_regex_ = std::regex(R"((password|passwd|pwd|secret|token)[:\s=]+['\"]?([^\s'\";,]{8,})['\"]?)", std::regex::icase);
}

ClassificationResult Classifier::classify_file(const std::string& file_path) {
    std::string content = read_file(file_path);

    if (content.empty()) {
        return ClassificationResult();
    }

    return classify_text(content);
}

ClassificationResult Classifier::classify_text(const std::string& content) {
    ClassificationResult result;

    // Check for credit card numbers
    if (matches_pattern(content, pan_regex_)) {
        result.labels.push_back("PAN");
    }

    // Check for SSN
    if (matches_pattern(content, ssn_regex_)) {
        result.labels.push_back("SSN");
    }

    // Check for email addresses
    if (matches_pattern(content, email_regex_)) {
        result.labels.push_back("EMAIL");
    }

    // Check for API keys
    if (matches_pattern(content, api_key_regex_)) {
        result.labels.push_back("API_KEY");
    }

    // Check for secrets
    if (matches_pattern(content, secret_regex_)) {
        result.labels.push_back("SECRET");
    }

    // Calculate confidence
    result.confidence = calculate_confidence(content, result.labels);

    return result;
}

std::string Classifier::read_file(const std::string& file_path) {
    try {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            Logger::warning("Could not open file: " + file_path);
            return "";
        }

        // Check file size (limit to 10MB)
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        const std::streamsize max_size = 10 * 1024 * 1024; // 10MB
        if (size > max_size) {
            Logger::warning("File too large, skipping: " + file_path);
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();

    } catch (const std::exception& e) {
        Logger::error("Error reading file " + file_path + ": " + e.what());
        return "";
    }
}

bool Classifier::matches_pattern(const std::string& content, const std::regex& pattern) {
    return std::regex_search(content, pattern);
}

double Classifier::calculate_confidence(const std::string& content,
                                       const std::vector<std::string>& labels) {
    if (labels.empty()) {
        return 0.0;
    }

    // Base confidence on number of matches
    double confidence = 0.7;

    // Increase confidence for multiple label types
    if (labels.size() > 1) {
        confidence += 0.1 * (labels.size() - 1);
    }

    // Cap at 0.95
    return std::min(confidence, 0.95);
}

} // namespace cybersentinel
