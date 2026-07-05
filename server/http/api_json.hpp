#pragma once

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

inline Json::Value errorJson(const std::string& error, const std::string& message) {
    Json::Value value;
    value["error"] = error;
    value["message"] = message;
    return value;
}

}  // namespace tiny_docs
