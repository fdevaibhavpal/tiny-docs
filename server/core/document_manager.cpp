#include "core/document_manager.hpp"

#include <chrono>
#include <filesystem>
#include <mutex>
#include <random>
#include <stdexcept>
#include <utility>

#include <poppler/cpp/poppler-page.h>

namespace tiny_docs {

namespace {

DocId randomId() {
    static std::mt19937_64 rng(
        static_cast<std::mt19937_64::result_type>(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    return "doc_" + std::to_string(rng());
}

std::string titleForPath(const std::string& path) {
    return std::filesystem::path(path).filename().string();
}

}  // namespace

DocId DocumentManager::makeId() const {
    return randomId();
}

DocId DocumentManager::open(const std::string& path) {
    std::unique_ptr<poppler::document> pdf(poppler::document::load_from_file(path));
    if (!pdf) {
        throw std::runtime_error("invalid_pdf");
    }
    if (pdf->is_encrypted()) {
        throw std::runtime_error("encrypted_pdf");
    }

    DocumentMeta meta;
    meta.id = makeId();
    meta.path = path;
    meta.title = titleForPath(path);
    meta.pageCount = pdf->pages();

    for (int pageIndex = 0; pageIndex < meta.pageCount; ++pageIndex) {
        auto page = pdf->create_page(pageIndex);
        if (!page) {
            throw std::runtime_error("invalid_pdf_page");
        }
        const auto rect = page->page_rect();
        meta.pageSizes.push_back(PageSize{
            static_cast<int>(rect.width()),
            static_cast<int>(rect.height()),
        });
    }

    auto document = std::make_unique<Document>(meta, std::move(pdf));
    const auto id = meta.id;

    std::unique_lock lock(mutex_);
    docs_[id] = std::move(document);
    return id;
}

Document& DocumentManager::get(const DocId& id) {
    std::shared_lock lock(mutex_);
    auto it = docs_.find(id);
    if (it == docs_.end()) {
        throw std::out_of_range("unknown_doc");
    }
    return *it->second;
}

const Document& DocumentManager::get(const DocId& id) const {
    std::shared_lock lock(mutex_);
    auto it = docs_.find(id);
    if (it == docs_.end()) {
        throw std::out_of_range("unknown_doc");
    }
    return *it->second;
}

void DocumentManager::close(const DocId& id) {
    std::unique_lock lock(mutex_);
    docs_.erase(id);
}

}  // namespace tiny_docs
