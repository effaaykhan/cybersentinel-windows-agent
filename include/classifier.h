#ifndef CYBERSENTINEL_CLASSIFIER_H
#define CYBERSENTINEL_CLASSIFIER_H

#include <string>
#include <vector>
#include <regex>

namespace cybersentinel {

struct ClassificationResult {
    std::vector<std::string> labels;
    double confidence;

    ClassificationResult() : confidence(0.0) {}
};

class Classifier {
public:
    Classifier();
    ~Classifier() = default;

    // Classify file content
    ClassificationResult classify_file(const std::string& file_path);

    // Classify text content
    ClassificationResult classify_text(const std::string& content);

private:
    // Pattern matchers
    std::regex pan_regex_;      // Credit card numbers
    std::regex ssn_regex_;      // Social Security Numbers
    std::regex email_regex_;    // Email addresses
    std::regex api_key_regex_;  // API keys
    std::regex secret_regex_;   // Generic secrets

    // Helper methods
    std::string read_file(const std::string& file_path);
    bool matches_pattern(const std::string& content, const std::regex& pattern);
    double calculate_confidence(const std::string& content,
                                const std::vector<std::string>& labels);
};

} // namespace cybersentinel

#endif // CYBERSENTINEL_CLASSIFIER_H
