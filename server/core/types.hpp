#pragma once

#include <array>
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

struct Hit {
    int page {};
    std::vector<std::array<double, 4>> rects;
    std::string snippet;
};

struct Annotation {
    std::string id;
    int page {};
    std::string type;
    std::string color;
    std::vector<std::array<double, 4>> rects;
    std::vector<std::array<double, 8>> quadPoints;
    std::string createdAt;
};

struct DocumentMeta {
    DocId id;
    std::string path;
    std::string title;
    int pageCount {};
    std::vector<PageSize> pageSizes;
};

}  // namespace tiny_docs
