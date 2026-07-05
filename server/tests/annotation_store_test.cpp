#include <filesystem>
#include <gtest/gtest.h>

#include "core/annotation_store.hpp"

namespace fs = std::filesystem;
using namespace tiny_docs;

TEST(AnnotationStore, AddFlushReloadAndRemove) {
    const fs::path path = fs::temp_directory_path() / "tiny_docs_annotations.json";
    std::error_code ec;
    fs::remove(path, ec);

    Annotation highlight;
    highlight.id = "ann_1";
    highlight.page = 1;
    highlight.type = "highlight";
    highlight.color = "#2E6B3B";
    highlight.rects.push_back({72.0, 100.0, 120.0, 18.0});
    highlight.quadPoints.push_back({72.0, 118.0, 192.0, 118.0, 192.0, 100.0, 72.0, 100.0});
    highlight.createdAt = "2026-07-05T10:00:00Z";

    {
        AnnotationStore store(path.string());
        store.add(highlight);
        store.flush();

        const auto all = store.all();
        ASSERT_EQ(all.size(), 1u);
        EXPECT_EQ(all.front().id, "ann_1");
    }

    {
        AnnotationStore store(path.string());
        const auto all = store.all();
        ASSERT_EQ(all.size(), 1u);
        EXPECT_EQ(all.front().color, "#2E6B3B");

        store.remove("ann_1");
        EXPECT_TRUE(store.all().empty());
    }

    fs::remove(path, ec);
}
