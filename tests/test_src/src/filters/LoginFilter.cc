#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpAppFramework.h>
#include <drogon/HttpTypes.h> // For HTTP status codes like k400BadRequest
#include <json/json.h>       // For JSON handling
#include <stdexcept>         // For std::runtime_error
#include <memory>            // For std::shared_ptr
#include "../filters/LoginFilter.h"
#include "../plugins/JwtPlugin.h"

using namespace drogon;
using ::testing::_;
using ::testing::Return;
using ::testing::Throw;
using ::testing::NiceMock;

// Mock for JwtPlugin
class MockJwtPlugin : public JwtPlugin {
public:
    MOCK_METHOD(jwt::jwt_object, init, (), (const, override));
};

// Mock for jwt::jwt_object
class MockJwtObject {
public:
    MOCK_METHOD(jwt::decoded_jwt, decode, (const std::string&), (const));
};

// Mock for jwt::decoded_jwt
class MockDecodedJwt {
public:
    MOCK_METHOD(std::string, get_payload_claim, (const std::string&), (const));
};

// Test fixture for LoginFilter
class LoginFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Mock the JwtPlugin and register it with the Drogon app
        jwtPluginMock = std::make_shared<NiceMock<MockJwtPlugin>>();
        drogon::app().registerPlugin(jwtPluginMock);
    }

    void TearDown() override {
        // Clear the registered plugins after each test
        drogon::app().clearPlugins();
    }

    std::shared_ptr<NiceMock<MockJwtPlugin>> jwtPluginMock;
};

// Test case: Missing Authorization header
TEST_F(LoginFilterTest, MissingAuthorizationHeader) {
    // Arrange
    LoginFilter filter;
    auto req = HttpRequest::newHttpRequest();
    bool callbackCalled = false;

    // Act
    filter.doFilter(req,
        [&](const HttpResponsePtr &resp) {
            // Assert
            ASSERT_EQ(resp->getStatusCode(), k400BadRequest);
            auto json = resp->getJsonObject();
            ASSERT_NE(json, nullptr);
            EXPECT_EQ((*json)["error"].asString(), "missing Authorization header");
            callbackCalled = true;
        },
        []() {
            FAIL() << "FilterChainCallback should not be called";
        });

    EXPECT_TRUE(callbackCalled);
}

// Test case: Invalid JWT token
TEST_F(LoginFilterTest, InvalidJwtToken) {
    // Arrange
    LoginFilter filter;
    auto req = HttpRequest::newHttpRequest();
    req->addHeader("Authorization", "Bearer invalid_token");

    MockJwtObject mockJwtObject;
    EXPECT_CALL(*jwtPluginMock, init())
        .WillOnce(Return(mockJwtObject));
    EXPECT_CALL(mockJwtObject, decode("invalid_token"))
        .WillOnce(Throw(jwt::token_verification_exception("Invalid token")));

    bool callbackCalled = false;

    // Act
    filter.doFilter(req,
        [&](const HttpResponsePtr &resp) {
            // Assert
            ASSERT_EQ(resp->getStatusCode(), k400BadRequest);
            auto json = resp->getJsonObject();
            ASSERT_NE(json, nullptr);
            EXPECT_EQ((*json)["error"].asString(), "invalid token");
            callbackCalled = true;
        },
        []() {
            FAIL() << "FilterChainCallback should not be called";
        });

    EXPECT_TRUE(callbackCalled);
}

// Test case: Valid JWT token
TEST_F(LoginFilterTest, ValidJwtToken) {
    // Arrange
    LoginFilter filter;
    auto req = HttpRequest::newHttpRequest();
    req->addHeader("Authorization", "Bearer valid_token");

    MockJwtObject mockJwtObject;
    MockDecodedJwt mockDecodedJwt;

    EXPECT_CALL(*jwtPluginMock, init())
        .WillOnce(Return(mockJwtObject));
    EXPECT_CALL(mockJwtObject, decode("valid_token"))
        .WillOnce(Return(mockDecodedJwt));
    EXPECT_CALL(mockDecodedJwt, get_payload_claim("user_id"))
        .WillOnce(Return("123"));

    bool chainCallbackCalled = false;

    // Act
    filter.doFilter(req,
        [](const HttpResponsePtr &resp) {
            FAIL() << "FilterCallback should not be called";
        },
        [&]() {
            // Assert
            chainCallbackCalled = true;
        });

    EXPECT_TRUE(chainCallbackCalled);
}

// Test case: Runtime error during processing
TEST_F(LoginFilterTest, RuntimeError) {
    // Arrange
    LoginFilter filter;
    auto req = HttpRequest::newHttpRequest();
    req->addHeader("Authorization", "Bearer valid_token");

    MockJwtObject mockJwtObject;
    EXPECT_CALL(*jwtPluginMock, init())
        .WillOnce(Return(mockJwtObject));
    EXPECT_CALL(mockJwtObject, decode("valid_token"))
        .WillOnce(Throw(std::runtime_error("Unexpected error")));

    bool callbackCalled = false;

    // Act
    filter.doFilter(req,
        [&](const HttpResponsePtr &resp) {
            // Assert
            ASSERT_EQ(resp->getStatusCode(), k500InternalServerError);
            auto json = resp->getJsonObject();
            ASSERT_NE(json, nullptr);
            EXPECT_EQ((*json)["error"].asString(), "unexpected error");
            callbackCalled = true;
        },
        []() {
            FAIL() << "FilterChainCallback should not be called";
        });

    EXPECT_TRUE(callbackCalled);
}