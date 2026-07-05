#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>

#include "core/document_manager.hpp"
#include "engine/annotation_writer.hpp"

namespace fs = std::filesystem;
using namespace tiny_docs;

namespace {

std::string makeTextPdf() {
    const std::string contents = "BT\n/F1 24 Tf\n72 100 Td\n(HELLO ANNOTATION) Tj\nET\n";

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

Annotation makeHighlight() {
    Annotation highlight;
    highlight.id = "ann_export_1";
    highlight.page = 1;
    highlight.type = "highlight";
    highlight.color = "#2E6B3B";
    highlight.rects.push_back({72.0, 100.0, 120.0, 18.0});
    highlight.quadPoints.push_back({72.0, 118.0, 192.0, 118.0, 192.0, 100.0, 72.0, 100.0});
    highlight.createdAt = "2026-07-05T10:00:00Z";
    return highlight;
}

int annotationCount(const fs::path& pdfPath) {
    QPDF qpdf;
    qpdf.processFile(pdfPath.c_str());
    auto pages = QPDFPageDocumentHelper(qpdf).getAllPages();
    auto annots = pages.at(0).getObjectHandle().getKey("/Annots");
    return annots.isArray() ? annots.getArrayNItems() : 0;
}

}  // namespace

TEST(AnnotationWriter, ExportsHighlightToNewPdf) {
    TempPdfFile source("tiny_docs_annotation_writer_source.pdf", makeTextPdf());
    const fs::path output = fs::temp_directory_path() / "tiny_docs_annotation_writer_output.annotated.pdf";
    std::error_code ec;
    fs::remove(output, ec);

    DocumentManager manager;
    const DocId id = manager.open(source.path().string());
    auto& document = manager.get(id);

    AnnotationWriter writer;
    const auto exported = writer.exportAnnotated(document, {makeHighlight()}, output.string());

    EXPECT_EQ(exported, output.string());
    EXPECT_TRUE(fs::exists(output));
    EXPECT_GT(fs::file_size(output), 0u);
    EXPECT_EQ(annotationCount(output), 1);
    EXPECT_EQ(annotationCount(source.path()), 0);

    fs::remove(output, ec);
}

TEST(AnnotationWriter, RejectsSamePathExport) {
    TempPdfFile source("tiny_docs_annotation_writer_same_path.pdf", makeTextPdf());

    DocumentManager manager;
    const DocId id = manager.open(source.path().string());
    auto& document = manager.get(id);

    AnnotationWriter writer;
    EXPECT_THROW(writer.exportAnnotated(document, {makeHighlight()}, source.path().string()), std::invalid_argument);
}
