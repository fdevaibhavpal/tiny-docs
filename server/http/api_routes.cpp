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

    app().registerHandler(
        "/documents/{1}/pages/{2}/render",
        [state](const HttpRequestPtr& request,
                std::function<void(const HttpResponsePtr&)>&& callback,
                const std::string& docId,
                int pageNumber) {
            try {
                auto& document = state->documentManager.get(docId);
                double dpi = 144.0;
                const auto dpiText = request->getParameter("dpi");
                if (!dpiText.empty()) {
                    dpi = std::stod(dpiText);
                }
                const auto png = state->renderer.renderPage(document, pageNumber, dpi);

                auto response = HttpResponse::newHttpResponse();
                response->setStatusCode(k200OK);
                response->setContentTypeCode(CT_IMAGE_PNG);
                response->setBody(std::string(reinterpret_cast<const char*>(png.data()), png.size()));
                callback(response);
            } catch (const std::out_of_range&) {
                callback(jsonResponse(errorJson("not_found", "document or page not found"),
                                      k404NotFound));
            } catch (const std::exception& ex) {
                callback(jsonResponse(errorJson("bad_request", ex.what()), k400BadRequest));
            }
        },
        {Get});

    app().registerHandler(
        "/documents/{1}/search",
        [state](const HttpRequestPtr& request,
                std::function<void(const HttpResponsePtr&)>&& callback,
                const std::string& docId) {
            const auto query = request->getParameter("q");
            if (query.empty()) {
                callback(jsonResponse(errorJson("bad_request", "q must be a non-empty string"),
                                      k400BadRequest));
                return;
            }

            try {
                auto& document = state->documentManager.get(docId);
                Json::Value value;
                value["query"] = query;
                value["hits"] = Json::arrayValue;
                for (const auto& hit : state->searchEngine.search(document, state->textIndex, query)) {
                    value["hits"].append(toJson(hit));
                }
                callback(jsonResponse(value));
            } catch (const std::out_of_range&) {
                callback(jsonResponse(errorJson("not_found", "unknown document"), k404NotFound));
            }
        },
        {Get});

    app().registerHandler(
        "/documents/{1}/annotations",
        [state](const HttpRequestPtr&,
                std::function<void(const HttpResponsePtr&)>&& callback,
                const std::string& docId) {
            auto store = state->findAnnotationStore(docId);
            if (!store) {
                callback(jsonResponse(errorJson("not_found", "unknown document"), k404NotFound));
                return;
            }

            Json::Value value;
            value["annotations"] = Json::arrayValue;
            for (const auto& annotation : store->all()) {
                value["annotations"].append(toJson(annotation));
            }
            callback(jsonResponse(value));
        },
        {Get});

    app().registerHandler(
        "/documents/{1}/annotations",
        [state](const HttpRequestPtr& request,
                std::function<void(const HttpResponsePtr&)>&& callback,
                const std::string& docId) {
            auto store = state->findAnnotationStore(docId);
            if (!store) {
                callback(jsonResponse(errorJson("not_found", "unknown document"), k404NotFound));
                return;
            }

            const auto json = request->getJsonObject();
            if (!json) {
                callback(jsonResponse(errorJson("bad_request", "annotation payload is incomplete"),
                                      k400BadRequest));
                return;
            }

            try {
                const auto annotation = annotationFromJson(*json);
                if (annotation.type != "highlight") {
                    callback(jsonResponse(errorJson("bad_request", "unsupported annotation type"),
                                          k400BadRequest));
                    return;
                }
                store->add(annotation);
                store->flush();

                Json::Value value;
                value["ok"] = true;
                value["annotationId"] = annotation.id;
                callback(jsonResponse(value));
            } catch (const std::invalid_argument& ex) {
                callback(jsonResponse(errorJson("bad_request", ex.what()), k400BadRequest));
            }
        },
        {Post});

    app().registerHandler(
        "/documents/{1}/annotations/{2}",
        [state](const HttpRequestPtr&,
                std::function<void(const HttpResponsePtr&)>&& callback,
                const std::string& docId,
                const std::string& annotationId) {
            auto store = state->findAnnotationStore(docId);
            if (!store) {
                callback(jsonResponse(errorJson("not_found", "unknown document"), k404NotFound));
                return;
            }

            store->remove(annotationId);
            store->flush();

            Json::Value value;
            value["ok"] = true;
            value["annotationId"] = annotationId;
            callback(jsonResponse(value));
        },
        {Delete});
}

}  // namespace tiny_docs
