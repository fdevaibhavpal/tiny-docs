#include <cstdlib>
#include <memory>

#include <drogon/drogon.h>

#include "http/api_routes.hpp"
#include "http/app_state.hpp"

int main() {
    auto state = std::make_shared<tiny_docs::AppState>();
    tiny_docs::registerApiRoutes(state);

    const char* host = std::getenv("TINY_DOCS_HOST");
    const char* portText = std::getenv("TINY_DOCS_PORT");
    const auto port = portText ? std::atoi(portText) : 8848;

    drogon::app().addListener(host ? host : "127.0.0.1", port);
    drogon::app().setThreadNum(1);
    drogon::app().run();
    return 0;
}
