#pragma once

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/document.hpp"

namespace tiny_docs {

class TextIndex {
public:
    std::vector<Span> getSpans(Document& document, int pageNumber);

private:
    std::unordered_map<std::string, std::vector<Span>> cache_;
    std::shared_mutex mutex_;
};

}  // namespace tiny_docs
