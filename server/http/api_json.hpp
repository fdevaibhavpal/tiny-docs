#pragma once

#include <array>
#include <stdexcept>
#include <string>

#include <json/value.h>

#include "core/types.hpp"

namespace tiny_docs {

inline Json::Value toJson(const PageSize& pageSize) {
    Json::Value value;
    value["width"] = pageSize.width;
    value["height"] = pageSize.height;
    return value;
}

inline Json::Value toJson(const DocumentMeta& meta) {
    Json::Value value;
    value["docId"] = meta.id;
    value["path"] = meta.path;
    value["title"] = meta.title;
    value["pageCount"] = meta.pageCount;
    value["pageSizes"] = Json::arrayValue;
    for (const auto& pageSize : meta.pageSizes) {
        value["pageSizes"].append(toJson(pageSize));
    }
    return value;
}

inline Json::Value toJson(const Hit& hit) {
    Json::Value value;
    value["page"] = hit.page;
    value["rects"] = Json::arrayValue;
    for (const auto& rect : hit.rects) {
        Json::Value rectValue(Json::arrayValue);
        for (double number : rect) {
            rectValue.append(number);
        }
        value["rects"].append(rectValue);
    }
    value["snippet"] = hit.snippet;
    return value;
}

inline std::array<double, 4> parseRect(const Json::Value& value) {
    if (!value.isArray() || value.size() != 4U) {
        throw std::invalid_argument("rect must have 4 numbers");
    }
    return {
        value[0].asDouble(),
        value[1].asDouble(),
        value[2].asDouble(),
        value[3].asDouble(),
    };
}

inline std::array<double, 8> parseQuadPoints(const Json::Value& value) {
    if (!value.isArray() || value.size() != 8U) {
        throw std::invalid_argument("quadPoints must have 8 numbers");
    }
    return {
        value[0].asDouble(),
        value[1].asDouble(),
        value[2].asDouble(),
        value[3].asDouble(),
        value[4].asDouble(),
        value[5].asDouble(),
        value[6].asDouble(),
        value[7].asDouble(),
    };
}

inline Annotation annotationFromJson(const Json::Value& value) {
    if (!value.isMember("id") || !value["id"].isString() ||
        !value.isMember("page") || !value["page"].isInt() ||
        !value.isMember("type") || !value["type"].isString() ||
        !value.isMember("color") || !value["color"].isString() ||
        !value.isMember("createdAt") || !value["createdAt"].isString() ||
        !value.isMember("rects") || !value["rects"].isArray() ||
        !value.isMember("quadPoints") || !value["quadPoints"].isArray()) {
        throw std::invalid_argument("annotation payload is incomplete");
    }

    Annotation annotation;
    annotation.id = value["id"].asString();
    annotation.page = value["page"].asInt();
    annotation.type = value["type"].asString();
    annotation.color = value["color"].asString();
    annotation.createdAt = value["createdAt"].asString();

    for (const auto& rect : value["rects"]) {
        annotation.rects.push_back(parseRect(rect));
    }
    for (const auto& quad : value["quadPoints"]) {
        annotation.quadPoints.push_back(parseQuadPoints(quad));
    }

    return annotation;
}

inline Json::Value toJson(const Annotation& annotation) {
    Json::Value value;
    value["id"] = annotation.id;
    value["page"] = annotation.page;
    value["type"] = annotation.type;
    value["color"] = annotation.color;
    value["createdAt"] = annotation.createdAt;
    value["rects"] = Json::arrayValue;
    for (const auto& rect : annotation.rects) {
        Json::Value rectValue(Json::arrayValue);
        for (double number : rect) {
            rectValue.append(number);
        }
        value["rects"].append(rectValue);
    }
    value["quadPoints"] = Json::arrayValue;
    for (const auto& quad : annotation.quadPoints) {
        Json::Value quadValue(Json::arrayValue);
        for (double number : quad) {
            quadValue.append(number);
        }
        value["quadPoints"].append(quadValue);
    }
    return value;
}

inline Json::Value errorJson(const std::string& error, const std::string& message) {
    Json::Value value;
    value["error"] = error;
    value["message"] = message;
    return value;
}

}  // namespace tiny_docs
