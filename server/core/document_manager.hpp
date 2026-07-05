#pragma once

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "core/document.hpp"

namespace tiny_docs {

class DocumentManager {
public:
    DocId open(const std::string& path);
    Document& get(const DocId& id);
    const Document& get(const DocId& id) const;
    void close(const DocId& id);

private:
    DocId makeId() const;

    mutable std::shared_mutex mutex_;
    std::unordered_map<DocId, std::unique_ptr<Document>> docs_;
};

}  // namespace tiny_docs
