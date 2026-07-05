#pragma once

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <future>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <cstdlib>

#include <drogon/drogon.h>

#include "http/api_routes.hpp"
#include "http/app_state.hpp"

namespace tiny_docs::test {

namespace fs = std::filesystem;

inline std::string makeMinimalPdf() {
    std::vector<std::string> objects = {
        "<< /Type /Catalog /Pages 2 0 R >>",
        "<< /Type /Pages /Kids [3 0 R] /Count 1 >>",
        "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 300 144] /Contents 4 0 R >>",
        "<< /Length 0 >>\nstream\n\nendstream"
    };

    std::ostringstream out;
    out << "%PDF-1.4\n";
    std::vector<long> offsets;
    offsets.push_back(0);

    for (size_t i = 0; i < objects.size(); ++i) {
        offsets.push_back(static_cast<long>(out.tellp()));
        out << (i + 1) << " 0 obj\n" << objects[i] << "\nendobj\n";
    }

    const long xrefOffset = static_cast<long>(out.tellp());
    out << "xref\n0 " << (objects.size() + 1) << "\n";
    out << "0000000000 65535 f \n";
    for (size_t i = 1; i < offsets.size(); ++i) {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%010ld 00000 n \n", offsets[i]);
        out << buffer;
    }
    out << "trailer\n<< /Size " << (objects.size() + 1) << " /Root 1 0 R >>\n";
    out << "startxref\n" << xrefOffset << "\n%%EOF\n";
    return out.str();
}

class TempPdfFile {
public:
    TempPdfFile(std::string name, std::string contents) {
        path_ = fs::temp_directory_path() / std::move(name);
        std::ofstream out(path_, std::ios::binary);
        out << contents;
    }

    ~TempPdfFile() {
        std::error_code ec;
        fs::remove(path_, ec);
    }

    const fs::path& path() const { return path_; }

private:
    fs::path path_;
};

struct Harness {
    std::shared_ptr<AppState> state;

    drogon::HttpResponsePtr postJson(const std::string& path, const Json::Value& body) const {
        auto client = drogon::HttpClient::newHttpClient("http://127.0.0.1:8849");
        auto request = drogon::HttpRequest::newHttpJsonRequest(body);
        request->setMethod(drogon::Post);
        request->setPath(path);
        std::promise<drogon::HttpResponsePtr> responsePromise;
        auto responseFuture = responsePromise.get_future();
        client->sendRequest(
            request,
            [&responsePromise](drogon::ReqResult result, const drogon::HttpResponsePtr& response) mutable {
                if (result != drogon::ReqResult::Ok) {
                    responsePromise.set_value(nullptr);
                    return;
                }
                responsePromise.set_value(response);
            });
        return responseFuture.get();
    }
};

Harness makeHarness();

inline Harness makeHarness() {
    static auto state = std::make_shared<AppState>();
    static std::once_flag once;
    static std::thread serverThread;

    std::call_once(once, [&] {
        registerApiRoutes(state);
        drogon::app().setThreadNum(1);
        drogon::app().addListener("127.0.0.1", 8849);
        serverThread = std::thread([] { drogon::app().run(); });
        std::atexit([] {
            drogon::app().quit();
            if (serverThread.joinable()) {
                serverThread.join();
            }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    return Harness{state};
}

}  // namespace tiny_docs::test
