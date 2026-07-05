#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "core/types.hpp"

namespace tiny_docs {

class AnnotationStore {
public:
    explicit AnnotationStore(std::string path);

    void add(const Annotation& annotation);
    void remove(const std::string& id);
    std::vector<Annotation> all() const;
    void flush() const;

private:
    void load();

    std::string path_;
    std::unordered_map<std::string, Annotation> items_;
};

}  // namespace tiny_docs
