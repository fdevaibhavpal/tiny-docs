#include "core/annotation_store.hpp"

#include <fstream>
#include <utility>

#include <nlohmann/json.hpp>

namespace tiny_docs {

using json = nlohmann::json;

namespace {

json toJson(const Annotation& annotation) {
    return json{
        {"id", annotation.id},
        {"page", annotation.page},
        {"type", annotation.type},
        {"color", annotation.color},
        {"rects", annotation.rects},
        {"quadPoints", annotation.quadPoints},
        {"createdAt", annotation.createdAt},
    };
}

Annotation fromJson(const json& value) {
    Annotation annotation;
    annotation.id = value.at("id").get<std::string>();
    annotation.page = value.at("page").get<int>();
    annotation.type = value.value("type", std::string{});
    annotation.color = value.value("color", std::string{});
    annotation.rects = value.value("rects", std::vector<std::array<double, 4>>{});
    annotation.quadPoints = value.value("quadPoints", std::vector<std::array<double, 8>>{});
    annotation.createdAt = value.value("createdAt", std::string{});
    return annotation;
}

}  // namespace

AnnotationStore::AnnotationStore(std::string path) : path_(std::move(path)) {
    load();
}

void AnnotationStore::add(const Annotation& annotation) {
    items_[annotation.id] = annotation;
}

void AnnotationStore::remove(const std::string& id) {
    items_.erase(id);
}

std::vector<Annotation> AnnotationStore::all() const {
    std::vector<Annotation> values;
    values.reserve(items_.size());
    for (const auto& [_, annotation] : items_) {
        values.push_back(annotation);
    }
    return values;
}

void AnnotationStore::flush() const {
    json root;
    root["annotations"] = json::array();
    for (const auto& [_, annotation] : items_) {
        root["annotations"].push_back(toJson(annotation));
    }

    std::ofstream out(path_);
    out << root.dump(2);
}

void AnnotationStore::load() {
    std::ifstream input(path_);
    if (!input) {
        return;
    }

    json root;
    input >> root;
    for (const auto& entry : root.value("annotations", json::array())) {
        auto annotation = fromJson(entry);
        items_[annotation.id] = std::move(annotation);
    }
}

}  // namespace tiny_docs
