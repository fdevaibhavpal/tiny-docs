#include "http/api_routes.hpp"

#include <exception>
#include <functional>

#include <drogon/drogon.h>

#include "http/api_json.hpp"
#include "http/app_state.hpp"

namespace tiny_docs {

namespace {

drogon::HttpResponsePtr jsonResponse(const Json::Value& value,
                                     drogon::HttpStatusCode statusCode = drogon::k200OK) {
    auto response = drogon::HttpResponse::newHttpJsonResponse(value);
    response->setStatusCode(statusCode);
    return response;
}

}  // namespace

void registerApiRoutes(const std::shared_ptr<AppState>& state) {
    using namespace drogon;

    app().registerHandler(
        "/documents/open",
        [state](const HttpRequestPtr& request,
                std::function<void(const HttpResponsePtr&)>&& callback) {
            const auto json = request->getJsonObject();
            if (!json || !json->isMember("path") || !(*json)["path"].isString()) {
                callback(jsonResponse(errorJson("bad_request", "path must be a string"),
                                      k400BadRequest));
                return;
            }

            try {
                const auto docId = state->documentManager.open((*json)["path"].asString());
                auto& document = state->documentManager.get(docId);
                state->ensureAnnotationStore(docId);
                callback(jsonResponse(toJson(document.meta())));
            } catch (const std::exception& ex) {
                callback(jsonResponse(errorJson("bad_request", ex.what()), k400BadRequest));
            }
        },
        {Post});
}

}  // namespace tiny_docs
