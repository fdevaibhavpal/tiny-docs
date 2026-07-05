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
