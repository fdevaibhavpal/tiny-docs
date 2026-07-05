#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/document_manager.hpp"
#include "engine/renderer.hpp"

namespace fs = std::filesystem;
using namespace tiny_docs;

namespace {

std::string makeMinimalPdf() {
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
        path_ = fs::temp_directory_path() / name;
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

}  // namespace

TEST(Renderer, RendersPageToPngBytes) {
    TempPdfFile pdf("tiny_docs_renderer_valid.pdf", makeMinimalPdf());
    DocumentManager manager;

    const DocId id = manager.open(pdf.path().string());
    auto& document = manager.get(id);

    Renderer renderer;
    const auto png = renderer.renderPage(document, 1, 144.0);

    ASSERT_GE(png.size(), 8u);
    EXPECT_EQ(static_cast<unsigned char>(png[0]), 0x89);
    EXPECT_EQ(static_cast<unsigned char>(png[1]), 0x50);
    EXPECT_EQ(static_cast<unsigned char>(png[2]), 0x4E);
    EXPECT_EQ(static_cast<unsigned char>(png[3]), 0x47);
}

TEST(Renderer, RejectsBadPageNumber) {
    TempPdfFile pdf("tiny_docs_renderer_invalid_page.pdf", makeMinimalPdf());
    DocumentManager manager;

    const DocId id = manager.open(pdf.path().string());
    auto& document = manager.get(id);

    Renderer renderer;
    EXPECT_THROW(renderer.renderPage(document, 2, 144.0), std::out_of_range);
}
