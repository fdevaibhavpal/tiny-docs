#pragma once

#include <cstdint>
#include <vector>

#include "core/document.hpp"

namespace tiny_docs {

class Renderer {
public:
    std::vector<std::uint8_t> renderPage(Document& document, int pageNumber, double dpi) const;
};

}  // namespace tiny_docs
