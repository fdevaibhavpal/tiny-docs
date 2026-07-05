#include "engine/renderer.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>

namespace fs = std::filesystem;

namespace tiny_docs {

std::vector<std::uint8_t> Renderer::renderPage(Document& document, int pageNumber, double dpi) const {
    if (pageNumber < 1 || pageNumber > document.meta().pageCount) {
        throw std::out_of_range("bad_page");
    }

    std::shared_lock lock(document.mutex());
    std::unique_ptr<poppler::page> page(document.pdf().create_page(pageNumber - 1));
    if (!page) {
        throw std::runtime_error("render_failed");
    }

    poppler::page_renderer renderer;
    const auto image = renderer.render_page(page.get(), dpi, dpi);
    if (!image.is_valid()) {
        throw std::runtime_error("render_failed");
    }

    const auto tempPath = fs::temp_directory_path() /
                          ("tiny_docs_render_" + std::to_string(pageNumber) + ".png");
    if (!image.save(tempPath.string(), "png")) {
        throw std::runtime_error("render_failed");
    }

    std::ifstream input(tempPath, std::ios::binary);
    std::vector<std::uint8_t> bytes(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    fs::remove(tempPath);

    return bytes;
}

}  // namespace tiny_docs
