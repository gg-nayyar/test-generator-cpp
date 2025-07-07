#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <drogon/drogon.h>
#include "LoginFilter.h"
#include "../plugins/JwtPlugin.h"

using namespace drogon;
using ::testing::_;
using ::testing::Return;
using ::testing::Throw;
using ::testing::NiceMock;

// Mock classes for dependencies
class MockHttpRequest : public HttpRequest {
public:
    MOCK_METHOD(std::string, getHeader, (const std::string &), (const, override));
};

class MockJwtPlugin : public JwtPlugin {
public:
    MOCK_METHOD(jwt::jwt_object, init, (), (const, override));
};

class MockJwtObject {
public:
    MOCK_METHOD(jwt::decoded_jwt, decode, (const std::string &), (const));
};

class MockDecodedJwt {
public:
    MOCK_METHOD(jwt::claim, get_payload_claim, (const std::string &), (const));
};

// Test suite for LoginFilter
class LoginFilterTest : public ::testing::Test {
protected:
    LoginFilter filter;
    NiceMock<MockHttpRequest> mockRequest;
    NiceMock<MockJwtPlugin> mockJwtPlugin;
    NiceMock<MockJwtObject> mockJwtObject;
    NiceMock<MockDecodedJwt> mockDecodedJwt;

    void SetUp() override {
        // Mock the plugin retrieval
        drogon::app().registerPlugin<MockJwtPlugin>(&mockJwtPlugin);
    }
};

TEST_F(LoginFilterTest, MissingAuthorizationHeader) {
    // Arrange
    EXPECT_CALL(mockRequest, getHeader("Authorization")).WillOnce(Return(""));

    bool callbackCalled = false;
    auto filterCallback = [&](const HttpResponsePtr &resp) {
        ASSERT_EQ(resp->getStatusCode(), k400BadRequest);
        callbackCalled = true;
    };

    auto chainCallback = [&]() {
        FAIL() << "Filter chain callback should not be called";
    };

    // Act
    filter.doFilter(mockRequest.shared_from_this(), filterCallback, chainCallback);

    // Assert
    ASSERT_TRUE(callbackCalled);
}

TEST_F(LoginFilterTest, InvalidTokenThrowsVerificationException) {
    // Arrange
    EXPECT_CALL(mockRequest, getHeader("Authorization")).WillOnce(Return("Bearer invalid_token"));
    EXPECT_CALL(mockJwtPlugin, init()).WillOnce(Return(mockJwtObject));
    EXPECT_CALL(mockJwtObject, decode("invalid_token")).WillOnce(Throw(jwt::token_verification_exception("Invalid token")));

    bool callbackCalled = false;
    auto filterCallback = [&](const HttpResponsePtr &resp) {
        ASSERT_EQ(resp->getStatusCode(), k400BadRequest);
        callbackCalled = true;
    };

    auto chainCallback = [&]() {
        FAIL() << "Filter chain callback should not be called";
    };

    // Act
    filter.doFilter(mockRequest.shared_from_this(), filterCallback, chainCallback);

    // Assert
    ASSERT_TRUE(callbackCalled);
}

TEST_F(LoginFilterTest, ValidTokenProceedsToNextFilter) {
    // Arrange
    EXPECT_CALL(mockRequest, getHeader("Authorization")).WillOnce(Return("Bearer valid_token"));
    EXPECT_CALL(mockJwtPlugin, init()).WillOnce(Return(mockJwtObject));
    EXPECT_CALL(mockJwtObject, decode("valid_token")).WillOnce(Return(mockDecodedJwt));
    EXPECT_CALL(mockDecodedJwt, get_payload_claim("user_id")).WillOnce(Return(jwt::claim(std::to_string(123))));

    bool chainCallbackCalled = false;
    auto filterCallback = [&](const HttpResponsePtr &resp) {
        FAIL() << "Filter callback should not be called";
    };

    auto chainCallback = [&]() {
        chainCallbackCalled = true;
    };

    // Act
    filter.doFilter(mockRequest.shared_from_this(), filterCallback, chainCallback);

    // Assert
    ASSERT_TRUE(chainCallbackCalled);
}

TEST_F(LoginFilterTest, RuntimeErrorCaught) {
    // Arrange
    EXPECT_CALL(mockRequest, getHeader("Authorization")).WillOnce(Return("Bearer valid_token"));
    EXPECT_CALL(mockJwtPlugin, init()).WillOnce(Return(mockJwtObject));
    EXPECT_CALL(mockJwtObject, decode("valid_token")).WillOnce(Throw(std::runtime_error("Runtime error")));

    bool callbackCalled = false;
    auto filterCallback = [&](const HttpResponsePtr &resp) {
        ASSERT_EQ(resp->getStatusCode(), k500InternalServerError);
        callbackCalled = true;
    };

    auto chainCallback = [&]() {
        FAIL() << "Filter chain callback should not be called";
    };

    // Act
    filter.doFilter(mockRequest.shared_from_this(), filterCallback, chainCallback);

    // Assert
    ASSERT_TRUE(callbackCalled);
}