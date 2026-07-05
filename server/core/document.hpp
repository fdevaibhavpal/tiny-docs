#pragma once

#include <memory>
#include <shared_mutex>

#include <poppler/cpp/poppler-document.h>

#include "core/types.hpp"

namespace tiny_docs {

class Document {
public:
    Document(DocumentMeta meta, std::unique_ptr<poppler::document> pdf);

    const DocumentMeta& meta() const;
    poppler::document& pdf();
    const poppler::document& pdf() const;
    std::shared_mutex& mutex();

private:
    DocumentMeta meta_;
    std::unique_ptr<poppler::document> pdf_;
    mutable std::shared_mutex mutex_;
};

}  // namespace tiny_docs
