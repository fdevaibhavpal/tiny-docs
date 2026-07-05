#include <gtest/gtest.h>

#include "tests/http_test_support.hpp"

TEST(RestApi, OpensDocumentAndReturnsMetadata) {
    tiny_docs::test::TempPdfFile pdf("tiny_docs_open_route.pdf", tiny_docs::test::makeMinimalPdf());
    const auto harness = tiny_docs::test::makeHarness();

    Json::Value body;
    body["path"] = pdf.path().string();

    const auto response = harness.postJson("/documents/open", body);

    ASSERT_EQ(response->statusCode(), drogon::k200OK);
    const auto json = response->getJsonObject();
    ASSERT_NE(json, nullptr);
    EXPECT_FALSE((*json)["docId"].asString().empty());
    EXPECT_EQ((*json)["path"].asString(), pdf.path().string());
    EXPECT_EQ((*json)["pageCount"].asInt(), 1);
    ASSERT_TRUE((*json)["pageSizes"].isArray());
    EXPECT_EQ((*json)["pageSizes"].size(), 1U);
}

TEST(RestApi, RendersPagePng) {
    tiny_docs::test::TempPdfFile pdf("tiny_docs_render_route.pdf", tiny_docs::test::makeMinimalPdf());
    const auto harness = tiny_docs::test::makeHarness();

    Json::Value body;
    body["path"] = pdf.path().string();
    const auto opened = harness.postJson("/documents/open", body);
    ASSERT_NE(opened, nullptr);
    const auto openedJson = opened->getJsonObject();
    ASSERT_NE(openedJson, nullptr);
    const auto docId = (*openedJson)["docId"].asString();

    const auto response = harness.get("/documents/" + docId + "/pages/1/render?dpi=144");

    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response->statusCode(), drogon::k200OK);
    EXPECT_EQ(response->contentType(), drogon::CT_IMAGE_PNG);
    ASSERT_GE(response->body().size(), 8u);
    EXPECT_EQ(static_cast<unsigned char>(response->body()[0]), 0x89);
    EXPECT_EQ(static_cast<unsigned char>(response->body()[1]), 0x50);
    EXPECT_EQ(static_cast<unsigned char>(response->body()[2]), 0x4E);
    EXPECT_EQ(static_cast<unsigned char>(response->body()[3]), 0x47);
}

TEST(RestApi, SearchesDocumentText) {
    tiny_docs::test::TempPdfFile pdf("tiny_docs_search_route.pdf", tiny_docs::test::makeTextPdf());
    const auto harness = tiny_docs::test::makeHarness();

    Json::Value body;
    body["path"] = pdf.path().string();
    const auto opened = harness.postJson("/documents/open", body);
    ASSERT_NE(opened, nullptr);
    const auto openedJson = opened->getJsonObject();
    ASSERT_NE(openedJson, nullptr);
    const auto docId = (*openedJson)["docId"].asString();

    const auto response = harness.get("/documents/" + docId + "/search?q=TEXTINDEX");

    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response->statusCode(), drogon::k200OK);
    const auto json = response->getJsonObject();
    ASSERT_NE(json, nullptr);
    EXPECT_EQ((*json)["query"].asString(), "TEXTINDEX");
    ASSERT_TRUE((*json)["hits"].isArray());
    ASSERT_FALSE((*json)["hits"].empty());
    EXPECT_EQ((*json)["hits"][0]["page"].asInt(), 1);
}

TEST(RestApi, CreatesListsAndDeletesAnnotations) {
    tiny_docs::test::TempPdfFile pdf("tiny_docs_annotations_route.pdf", tiny_docs::test::makeTextPdf());
    const auto harness = tiny_docs::test::makeHarness();

    Json::Value openBody;
    openBody["path"] = pdf.path().string();
    const auto opened = harness.postJson("/documents/open", openBody);
    ASSERT_NE(opened, nullptr);
    const auto openedJson = opened->getJsonObject();
    ASSERT_NE(openedJson, nullptr);
    const auto docId = (*openedJson)["docId"].asString();

    Json::Value annotation;
    annotation["id"] = "ann-1";
    annotation["page"] = 1;
    annotation["type"] = "highlight";
    annotation["color"] = "#FFF200";
    annotation["createdAt"] = "2026-07-05T12:00:00Z";
    Json::Value rect(Json::arrayValue);
    rect.append(10.0);
    rect.append(20.0);
    rect.append(30.0);
    rect.append(12.0);
    annotation["rects"].append(rect);
    Json::Value quad(Json::arrayValue);
    quad.append(10.0);
    quad.append(32.0);
    quad.append(40.0);
    quad.append(32.0);
    quad.append(10.0);
    quad.append(20.0);
    quad.append(40.0);
    quad.append(20.0);
    annotation["quadPoints"].append(quad);

    const auto created = harness.postJson("/documents/" + docId + "/annotations", annotation);
    ASSERT_NE(created, nullptr);
    ASSERT_EQ(created->statusCode(), drogon::k200OK);

    const auto listed = harness.get("/documents/" + docId + "/annotations");
    ASSERT_NE(listed, nullptr);
    ASSERT_EQ(listed->statusCode(), drogon::k200OK);
    const auto listedJson = listed->getJsonObject();
    ASSERT_NE(listedJson, nullptr);
    ASSERT_TRUE((*listedJson)["annotations"].isArray());
    ASSERT_EQ((*listedJson)["annotations"].size(), 1U);
    EXPECT_EQ((*listedJson)["annotations"][0]["id"].asString(), "ann-1");

    const auto removed = harness.del("/documents/" + docId + "/annotations/ann-1");
    ASSERT_NE(removed, nullptr);
    ASSERT_EQ(removed->statusCode(), drogon::k200OK);

    const auto afterDelete = harness.get("/documents/" + docId + "/annotations");
    ASSERT_NE(afterDelete, nullptr);
    const auto afterDeleteJson = afterDelete->getJsonObject();
    ASSERT_NE(afterDeleteJson, nullptr);
    ASSERT_TRUE((*afterDeleteJson)["annotations"].isArray());
    EXPECT_EQ((*afterDeleteJson)["annotations"].size(), 0U);
}
