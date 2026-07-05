#include "engine/text_index.hpp"

#include <memory>
#include <mutex>
#include <stdexcept>

#include <poppler/cpp/poppler-page.h>

namespace tiny_docs {

namespace {

std::string cacheKey(const Document& document, int pageNumber) {
    return document.meta().id + ":" + std::to_string(pageNumber);
}

}  // namespace

std::vector<Span> TextIndex::getSpans(Document& document, int pageNumber) {
    if (pageNumber < 1 || pageNumber > document.meta().pageCount) {
        throw std::out_of_range("bad_page");
    }

    const auto key = cacheKey(document, pageNumber);
    {
        std::shared_lock lock(mutex_);
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return it->second;
        }
    }

    std::vector<Span> spans;
    std::shared_lock lock(document.mutex());
    std::unique_ptr<poppler::page> page(document.pdf().create_page(pageNumber - 1));
    if (!page) {
        throw std::runtime_error("text_extract_failed");
    }

    for (const auto& box : page->text_list()) {
        const auto rect = box.bbox();
        Span span;
        span.text = box.text().to_latin1();
        span.x = rect.x();
        span.y = rect.y();
        span.width = rect.width();
        span.height = rect.height();
        if (box.has_font_info()) {
            span.font = box.get_font_name();
            span.size = box.get_font_size();
        }
        spans.push_back(std::move(span));
    }

    {
        std::unique_lock writeLock(mutex_);
        cache_[key] = spans;
    }
    return spans;
}

}  // namespace tiny_docs
