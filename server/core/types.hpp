#pragma once

#include <string>
#include <vector>

namespace tiny_docs {

using DocId = std::string;

struct PageSize {
    int width {};
    int height {};
};

struct Span {
    std::string text;
    double x {};
    double y {};
    double width {};
    double height {};
    std::string font;
    double size {};
};

struct DocumentMeta {
    DocId id;
    std::string path;
    std::string title;
    int pageCount {};
    std::vector<PageSize> pageSizes;
};

}  // namespace tiny_docs
