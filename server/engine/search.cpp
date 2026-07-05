#include "engine/search.hpp"

#include <algorithm>
#include <cctype>

namespace tiny_docs {

namespace {

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

}  // namespace

std::vector<Hit> SearchEngine::search(Document& document, TextIndex& index, const std::string& query) const {
    if (query.empty()) {
        return {};
    }

    const auto needle = lowercase(query);
    std::vector<Hit> hits;

    for (int pageNumber = 1; pageNumber <= document.meta().pageCount; ++pageNumber) {
        const auto spans = index.getSpans(document, pageNumber);
        for (const auto& span : spans) {
            const auto haystack = lowercase(span.text);
            if (haystack.find(needle) == std::string::npos) {
                continue;
            }

            Hit hit;
            hit.page = pageNumber;
            hit.snippet = span.text;
            hit.rects.push_back({span.x, span.y, span.width, span.height});
            hits.push_back(std::move(hit));
        }
    }

    return hits;
}

}  // namespace tiny_docs
