#include "core/document.hpp"

#include <utility>

namespace tiny_docs {

Document::Document(DocumentMeta meta, std::unique_ptr<poppler::document> pdf)
    : meta_(std::move(meta)), pdf_(std::move(pdf)) {}

const DocumentMeta& Document::meta() const {
    return meta_;
}

poppler::document& Document::pdf() {
    return *pdf_;
}

const poppler::document& Document::pdf() const {
    return *pdf_;
}

std::shared_mutex& Document::mutex() {
    return mutex_;
}

}  // namespace tiny_docs
