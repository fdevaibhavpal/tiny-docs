#pragma once

#include <string>
#include <vector>

#include "core/document.hpp"
#include "core/types.hpp"

namespace tiny_docs {

class AnnotationWriter {
public:
    std::string exportAnnotated(
        const Document& document,
        const std::vector<Annotation>& annotations,
        const std::string& outputPath) const;
};

}  // namespace tiny_docs
