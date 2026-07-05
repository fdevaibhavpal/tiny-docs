#pragma once

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "core/annotation_store.hpp"
#include "core/document_manager.hpp"
#include "engine/annotation_writer.hpp"
#include "engine/renderer.hpp"
#include "engine/search.hpp"
#include "engine/text_index.hpp"

namespace tiny_docs {

struct AppState {
    DocumentManager documentManager;
    Renderer renderer;
    TextIndex textIndex;
    SearchEngine searchEngine;
    AnnotationWriter annotationWriter;

    std::shared_ptr<AnnotationStore> ensureAnnotationStore(const DocId& docId) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = annotationStores.find(docId);
        if (it != annotationStores.end()) {
            return it->second;
        }

        auto store = std::make_shared<AnnotationStore>(annotationPathFor(docId));
        annotationStores.emplace(docId, store);
        return store;
    }

    std::shared_ptr<AnnotationStore> findAnnotationStore(const DocId& docId) const {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = annotationStores.find(docId);
        if (it == annotationStores.end()) {
            return nullptr;
        }
        return it->second;
    }

    void removeAnnotationStore(const DocId& docId) {
        std::lock_guard<std::mutex> lock(mutex);
        annotationStores.erase(docId);
    }

    std::string annotationPathFor(const DocId& docId) const {
        const auto dir = cacheRoot / "annotations";
        std::filesystem::create_directories(dir);
        return (dir / (docId + ".json")).string();
    }

    std::filesystem::path cacheRoot {std::filesystem::temp_directory_path() / "tiny_docs_server_cache"};

private:
    mutable std::mutex mutex;
    std::unordered_map<DocId, std::shared_ptr<AnnotationStore>> annotationStores;
};

}  // namespace tiny_docs
