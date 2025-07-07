
You are an AI C++ developer. Here's the C++ test file with build errors.
Your job is to fix all errors from the build log and return the corrected version only.

--- FILE START ---

You are an AI C++ developer. Here's the C++ test file with build errors.
Your job is to fix all errors from the build log and return the corrected version only.

--- FILE START ---

You are an AI C++ developer. Here's the C++ test file with build errors.
Your job is to fix all errors from the build log and return the corrected version only.

--- FILE START ---

You are an AI C++ developer. Here's the C++ test file with build errors.
Your job is to fix all errors from the build log and return the corrected version only.

--- FILE START ---

You are an AI C++ developer. Here's the C++ test file with build errors.
Your job is to fix all errors from the build log and return the corrected version only.

--- FILE START ---

You are an AI C++ developer. Here's the C++ test file with build errors.
Your job is to fix all errors from the build log and return the corrected version only.

--- FILE START ---

You are an AI C++ developer. Here's the C++ test file with build errors.
Your job is to fix all errors from the build log and return the corrected version only.

--- FILE START ---

You are an AI C++ developer. Here's the C++ test file with build errors.
Your job is to fix all errors from the build log and return the corrected version only.

--- FILE START ---

You are an AI C++ developer. Here's the C++ test file with build errors.
Your job is to fix all errors from the build log and return the corrected version only.

--- FILE START ---
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "AuthController.h"
#include "drogon/HttpAppFramework.h"
#include "drogon/HttpResponse.h"
#include "drogon/HttpRequest.h"
#include "drogon/orm/Mapper.h"
#include "drogon/orm/Criterion.h"
#include "drogon/orm/Criteria.h"
#include "plugins/JwtPlugin.h"
#include "models/User.h"

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::org_chart;
using testing::_;
using testing::Return;
using testing::Throw;

// Mock classes
class MockJwtPlugin : public JwtPlugin {
public:
    MOCK_METHOD(Jwt, init, (), (const, override));
};

class MockMapper : public Mapper<User> {
public:
    MockMapper() : Mapper(nullptr) {}
    MOCK_METHOD(Future<std::vector<User>>, findFutureBy, (const Criteria &), (override));
    MOCK_METHOD(Future<void>, insertFuture, (const User &), (override));
};

// Test fixture
class AuthControllerTest : public ::testing::Test {
protected:
    AuthController authController;
    MockJwtPlugin mockJwtPlugin;
    MockMapper mockMapper;

    void SetUp() override {
        // Mock the JwtPlugin
        drogon::app().registerPlugin<MockJwtPlugin>(&mockJwtPlugin);
    }
};

// Helper function to create a mock HttpRequest
HttpRequestPtr createMockRequest(const Json::Value &json) {
    auto req = HttpRequest::newHttpJsonRequest(json);
    return req;
}

// Helper function to capture HttpResponse
void captureResponse(const HttpResponsePtr &resp, HttpResponsePtr &capturedResp) {
    capturedResp = resp;
}

// Tests for registerUser
TEST_F(AuthControllerTest, RegisterUser_MissingFields) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    auto req = createMockRequest(json);
    HttpResponsePtr capturedResp;

    // Act
    authController.registerUser(req, [&](const HttpResponsePtr &resp) {
        captureResponse(resp, capturedResp);
    }, User(json));

    // Assert
    ASSERT_EQ(capturedResp->getStatusCode(), HttpStatusCode::k400BadRequest);
    auto responseJson = capturedResp->getJsonObject();
    ASSERT_EQ((*responseJson)["error"].asString(), "missing fields");
}

TEST_F(AuthControllerTest, RegisterUser_UsernameTaken) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    json["password"] = "password123";
    auto req = createMockRequest(json);
    HttpResponsePtr capturedResp;

    EXPECT_CALL(mockMapper, findFutureBy(_))
        .WillOnce(Return(std::vector<User>{User(json)}));

    // Act
    authController.registerUser(req, [&](const HttpResponsePtr &resp) {
        captureResponse(resp, capturedResp);
    }, User(json));

    // Assert
    ASSERT_EQ(capturedResp->getStatusCode(), HttpStatusCode::k400BadRequest);
    auto responseJson = capturedResp->getJsonObject();
    ASSERT_EQ((*responseJson)["error"].asString(), "username is taken");
}

TEST_F(AuthControllerTest, RegisterUser_Success) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    json["password"] = "password123";
    auto req = createMockRequest(json);
    HttpResponsePtr capturedResp;

    EXPECT_CALL(mockMapper, findFutureBy(_))
        .WillOnce(Return(std::vector<User>{}));
    EXPECT_CALL(mockMapper, insertFuture(_))
        .WillOnce(Return());

    // Act
    authController.registerUser(req, [&](const HttpResponsePtr &resp) {
        captureResponse(resp, capturedResp);
    }, User(json));

    // Assert
    ASSERT_EQ(capturedResp->getStatusCode(), HttpStatusCode::k201Created);
    auto responseJson = capturedResp->getJsonObject();
    ASSERT_TRUE(responseJson->isMember("username"));
    ASSERT_TRUE(responseJson->isMember("token"));
}

// Tests for loginUser
TEST_F(AuthControllerTest, LoginUser_MissingFields) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    auto req = createMockRequest(json);
    HttpResponsePtr capturedResp;

    // Act
    authController.loginUser(req, [&](const HttpResponsePtr &resp) {
        captureResponse(resp, capturedResp);
    }, User(json));

    // Assert
    ASSERT_EQ(capturedResp->getStatusCode(), HttpStatusCode::k400BadRequest);
    auto responseJson = capturedResp->getJsonObject();
    ASSERT_EQ((*responseJson)["error"].asString(), "missing fields");
}

TEST_F(AuthControllerTest, LoginUser_UserNotFound) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    json["password"] = "password123";
    auto req = createMockRequest(json);
    HttpResponsePtr capturedResp;

    EXPECT_CALL(mockMapper, findFutureBy(_))
        .WillOnce(Return(std::vector<User>{}));

    // Act
    authController.loginUser(req, [&](const HttpResponsePtr &resp) {
        captureResponse(resp, capturedResp);
    }, User(json));

    // Assert
    ASSERT_EQ(capturedResp->getStatusCode(), HttpStatusCode::k400BadRequest);
    auto responseJson = capturedResp->getJsonObject();
    ASSERT_EQ((*responseJson)["error"].asString(), "user not found");
}

TEST_F(AuthControllerTest, LoginUser_InvalidPassword) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    json["password"] = "wrong_password";
    auto req = createMockRequest(json);
    HttpResponsePtr capturedResp;

    User user;
    user.setUsername("test_user");
    user.setPassword("hashed_password");

    EXPECT_CALL(mockMapper, findFutureBy(_))
        .WillOnce(Return(std::vector<User>{user}));
    EXPECT_CALL(mockJwtPlugin, validatePassword(_, _))
        .WillOnce(Return(false));

    // Act
    authController.loginUser(req, [&](const HttpResponsePtr &resp) {
        captureResponse(resp, capturedResp);
    }, User(json));

    // Assert
    ASSERT_EQ(capturedResp->getStatusCode(), HttpStatusCode::k401Unauthorized);
    auto responseJson = capturedResp->getJsonObject();
    ASSERT_EQ((*responseJson)["error"].asString(), "username and password do not match");
}

TEST_F(AuthControllerTest, LoginUser_Success) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    json["password"] = "password123";
    auto req = createMockRequest(json);
    HttpResponsePtr capturedResp;

    User user;
    user.setUsername("test_user");
    user.setPassword("hashed_password");

    EXPECT_CALL(mockMapper, findFutureBy(_))
        .WillOnce(Return(std::vector<User>{user}));
    EXPECT_CALL(mockJwtPlugin, validatePassword(_, _))
        .WillOnce(Return(true));

    // Act
    authController.loginUser(req, [&](const HttpResponsePtr &resp) {
        captureResponse(resp, capturedResp);
    }, User(json));

    // Assert
    ASSERT_EQ(capturedResp->getStatusCode(), HttpStatusCode::k200OK);
    auto responseJson = capturedResp->getJsonObject();
    ASSERT_TRUE(responseJson->isMember("username"));
    ASSERT_TRUE(responseJson->isMember("token"));
}
--- FILE END ---

--- BUILD LOG ---
Build logs not found or empty.
--- LOG END ---

Fix all errors and return the corrected C++ code only.
  
--- FILE END ---

--- BUILD LOG ---
Build logs not found or empty.
--- LOG END ---

Fix all errors and return the corrected C++ code only.
  
--- FILE END ---

--- BUILD LOG ---
Build logs not found or empty.
--- LOG END ---

Fix all errors and return the corrected C++ code only.
  
--- FILE END ---

--- BUILD LOG ---
Build logs not found or empty.
--- LOG END ---

Fix all errors and return the corrected C++ code only.
  
--- FILE END ---

--- BUILD LOG ---
Build logs not found or empty.
--- LOG END ---

Fix all errors and return the corrected C++ code only.
  
--- FILE END ---

--- BUILD LOG ---
Build logs not found or empty.
--- LOG END ---

Fix all errors and return the corrected C++ code only.
  
--- FILE END ---

--- BUILD LOG ---
Build logs not found or empty.
--- LOG END ---

Fix all errors and return the corrected C++ code only.
  
--- FILE END ---

--- BUILD LOG ---
Build logs not found or empty.
--- LOG END ---

Fix all errors and return the corrected C++ code only.
  
--- FILE END ---

--- BUILD LOG ---
Build logs not found or empty.
--- LOG END ---

Fix all errors and return the corrected C++ code only.
  