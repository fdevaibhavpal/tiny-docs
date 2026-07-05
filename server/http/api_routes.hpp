#pragma once

#include <memory>

namespace tiny_docs {

struct AppState;

void registerApiRoutes(const std::shared_ptr<AppState>& state);

}  // namespace tiny_docs
