#pragma once

#include <string>
#include <vector>

#include "core/document.hpp"
#include "engine/text_index.hpp"

namespace tiny_docs {

class SearchEngine {
public:
    std::vector<Hit> search(Document& document, TextIndex& index, const std::string& query) const;
};

}  // namespace tiny_docs
