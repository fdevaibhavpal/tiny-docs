#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

#include "core/document_manager.hpp"
#include "engine/search.hpp"
#include "engine/text_index.hpp"

namespace fs = std::filesystem;
using namespace tiny_docs;

namespace {

std::string makeTextPdf() {
    const std::string contents = "BT\n/F1 24 Tf\n72 100 Td\n(HELLO TEXTINDEX) Tj\nET\n";

    std::vector<std::string> objects = {
        "<< /Type /Catalog /Pages 2 0 R >>",
        "<< /Type /Pages /Kids [3 0 R] /Count 1 >>",
        "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 300 144] /Resources << /Font << /F1 5 0 R >> >> /Contents 4 0 R >>",
        "<< /Length " + std::to_string(contents.size()) + " >>\nstream\n" + contents + "endstream",
        "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>"
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

TEST(SearchEngine, FindsKnownTerm) {
    TempPdfFile pdf("tiny_docs_search.pdf", makeTextPdf());
    DocumentManager manager;

    const DocId id = manager.open(pdf.path().string());
    auto& document = manager.get(id);

    TextIndex index;
    SearchEngine search;
    const auto hits = search.search(document, index, "TEXTINDEX");

    ASSERT_FALSE(hits.empty());
    EXPECT_EQ(hits.front().page, 1);
    EXPECT_FALSE(hits.front().snippet.empty());
    EXPECT_FALSE(hits.front().rects.empty());
}

TEST(SearchEngine, ReturnsEmptyForMiss) {
    TempPdfFile pdf("tiny_docs_search_miss.pdf", makeTextPdf());
    DocumentManager manager;

    const DocId id = manager.open(pdf.path().string());
    auto& document = manager.get(id);

    TextIndex index;
    SearchEngine search;
    const auto hits = search.search(document, index, "MISSING_TERM");

    EXPECT_TRUE(hits.empty());
}
