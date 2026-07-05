#include "engine/annotation_writer.hpp"

#include <filesystem>
#include <stdexcept>
#include <vector>

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFWriter.hh>

namespace fs = std::filesystem;

namespace tiny_docs {

namespace {

std::string defaultOutputPath(const std::string& inputPath) {
    fs::path path(inputPath);
    return (path.parent_path() /
            (path.stem().string() + ".annotated" + path.extension().string()))
        .string();
}

QPDFObjectHandle toRect(const std::array<double, 4>& rect) {
    return QPDFObjectHandle::newArray(std::vector<QPDFObjectHandle>{
        QPDFObjectHandle::newReal(rect[0]),
        QPDFObjectHandle::newReal(rect[1]),
        QPDFObjectHandle::newReal(rect[0] + rect[2]),
        QPDFObjectHandle::newReal(rect[1] + rect[3]),
    });
}

QPDFObjectHandle toQuadPoints(const std::array<double, 8>& quadPoints) {
    std::vector<QPDFObjectHandle> values;
    values.reserve(8);
    for (double value : quadPoints) {
        values.push_back(QPDFObjectHandle::newReal(value));
    }
    return QPDFObjectHandle::newArray(values);
}

QPDFObjectHandle toColor(const std::string& color) {
    if (color.size() != 7 || color[0] != '#') {
        return QPDFObjectHandle::newArray(std::vector<QPDFObjectHandle>{
            QPDFObjectHandle::newReal(1.0),
            QPDFObjectHandle::newReal(1.0),
            QPDFObjectHandle::newReal(0.0),
        });
    }

    auto channel = [&](int offset) {
        return static_cast<double>(std::stoi(color.substr(offset, 2), nullptr, 16)) / 255.0;
    };

    return QPDFObjectHandle::newArray(std::vector<QPDFObjectHandle>{
        QPDFObjectHandle::newReal(channel(1)),
        QPDFObjectHandle::newReal(channel(3)),
        QPDFObjectHandle::newReal(channel(5)),
    });
}

QPDFObjectHandle makeHighlightAnnotation(QPDF& qpdf, const Annotation& annotation) {
    if (annotation.rects.empty()) {
        throw std::invalid_argument("missing_rect");
    }
    if (annotation.quadPoints.empty()) {
        throw std::invalid_argument("missing_quad_points");
    }

    auto quadPoints = QPDFObjectHandle::newArray();
    for (const auto& item : annotation.quadPoints) {
        auto row = toQuadPoints(item);
        for (int i = 0; i < row.getArrayNItems(); ++i) {
            quadPoints.appendItem(row.getArrayItem(i));
        }
    }

    auto annot = QPDFObjectHandle::newDictionary();
    annot.replaceKey("/Type", QPDFObjectHandle::newName("/Annot"));
    annot.replaceKey("/Subtype", QPDFObjectHandle::newName("/Highlight"));
    annot.replaceKey("/Rect", toRect(annotation.rects.front()));
    annot.replaceKey("/QuadPoints", quadPoints);
    annot.replaceKey("/C", toColor(annotation.color));
    annot.replaceKey("/NM", QPDFObjectHandle::newString(annotation.id));
    annot.replaceKey("/M", QPDFObjectHandle::newString(annotation.createdAt));
    return qpdf.makeIndirectObject(annot);
}

}  // namespace

std::string AnnotationWriter::exportAnnotated(
    const Document& document,
    const std::vector<Annotation>& annotations,
    const std::string& outputPath) const {
    const auto finalOutputPath = outputPath.empty() ? defaultOutputPath(document.meta().path) : outputPath;

    if (fs::absolute(finalOutputPath) == fs::absolute(document.meta().path)) {
        throw std::invalid_argument("same_output_path");
    }

    QPDF qpdf;
    qpdf.processFile(document.meta().path.c_str());

    auto pages = QPDFPageDocumentHelper(qpdf).getAllPages();
    for (const auto& annotation : annotations) {
        if (annotation.type != "highlight") {
            throw std::invalid_argument("unsupported_annotation_type");
        }
        if (annotation.page < 1 || annotation.page > static_cast<int>(pages.size())) {
            throw std::out_of_range("bad_page");
        }

        auto pageObject = pages.at(annotation.page - 1).getObjectHandle();
        auto annots = pageObject.getKey("/Annots");
        if (!annots.isArray()) {
            annots = QPDFObjectHandle::newArray();
            pageObject.replaceKey("/Annots", annots);
        }
        annots.appendItem(makeHighlightAnnotation(qpdf, annotation));
    }

    QPDFWriter writer(qpdf, finalOutputPath.c_str());
    writer.write();
    return finalOutputPath;
}

}  // namespace tiny_docs
