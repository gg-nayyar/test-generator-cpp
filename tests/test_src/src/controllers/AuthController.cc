#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <future>
#include <memory>
#include <json/json.h>
#include <drogon/drogon.h>
#include <drogon/orm/Mapper.h>
#include <drogon/orm/Criterion.h>
#include "AuthController.h"
#include "plugins/JwtPlugin.h"

// Mock classes for dependencies
class MockJwtPlugin : public drogon::PluginBase {
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(drogon::Jwt, generateToken, (const Json::Value &payload), (const));
};

class User {
public:
    void setPassword(const std::string &password) {
        this->password = password;
    }

    std::string getPassword() const {
        return password;
    }

private:
    std::string password;
};

class MockMapper : public drogon::orm::Mapper<User> {
public:
    MockMapper() : drogon::orm::Mapper<User>(nullptr) {}

    MOCK_METHOD(std::future<std::vector<User>>, findFutureBy, (const drogon::orm::Criteria &), (const));
    MOCK_METHOD(std::future<void>, insertFuture, (const User &), (const));
};

// Test fixture for AuthController
class AuthControllerTest : public ::testing::Test {
protected:
    AuthController authController;
    std::shared_ptr<MockMapper> mockMapper;
    std::shared_ptr<MockJwtPlugin> mockJwtPlugin;

    void SetUp() override {
        mockMapper = std::make_shared<MockMapper>();
        mockJwtPlugin = std::make_shared<MockJwtPlugin>();
        drogon::app().registerPlugin(mockJwtPlugin);
    }
};

// Helper function to create a mock HTTP request
std::shared_ptr<drogon::HttpRequest> createMockRequest(const Json::Value &json) {
    auto req = drogon::HttpRequest::newHttpJsonRequest(json);
    req->setMethod(drogon::Post);
    return req;
}

// Test for registerUser: missing fields
TEST_F(AuthControllerTest, RegisterUser_MissingFields) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    auto req = createMockRequest(json);

    bool callbackCalled = false;
    drogon::HttpResponsePtr response;

    auto callback = [&](const drogon::HttpResponsePtr &resp) {
        callbackCalled = true;
        response = resp;
    };

    // Act
    authController.registerUser(req, callback);

    // Assert
    ASSERT_TRUE(callbackCalled);
    ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k400BadRequest);
    auto responseBody = response->getJsonObject();
    ASSERT_TRUE(responseBody);
    ASSERT_EQ((*responseBody)["error"].asString(), "missing fields");
}

// Test for registerUser: username is taken
TEST_F(AuthControllerTest, RegisterUser_UsernameTaken) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    json["password"] = "password123";
    auto req = createMockRequest(json);

    EXPECT_CALL(*mockMapper, findFutureBy(::testing::_))
        .WillOnce(::testing::Return(std::async(std::launch::deferred, []() {
            return std::vector<User>{User()};
        })));

    bool callbackCalled = false;
    drogon::HttpResponsePtr response;

    auto callback = [&](const drogon::HttpResponsePtr &resp) {
        callbackCalled = true;
        response = resp;
    };

    // Act
    authController.registerUser(req, callback);

    // Assert
    ASSERT_TRUE(callbackCalled);
    ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k400BadRequest);
    auto responseBody = response->getJsonObject();
    ASSERT_TRUE(responseBody);
    ASSERT_EQ((*responseBody)["error"].asString(), "username is taken");
}

// Test for registerUser: successful registration
TEST_F(AuthControllerTest, RegisterUser_Success) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    json["password"] = "password123";
    auto req = createMockRequest(json);

    EXPECT_CALL(*mockMapper, findFutureBy(::testing::_))
        .WillOnce(::testing::Return(std::async(std::launch::deferred, []() {
            return std::vector<User>{};
        })));

    EXPECT_CALL(*mockMapper, insertFuture(::testing::_))
        .WillOnce(::testing::Return(std::async(std::launch::deferred, []() {})));

    EXPECT_CALL(*mockJwtPlugin, generateToken(::testing::_))
        .WillOnce(::testing::Return(drogon::Jwt("mock_token")));

    bool callbackCalled = false;
    drogon::HttpResponsePtr response;

    auto callback = [&](const drogon::HttpResponsePtr &resp) {
        callbackCalled = true;
        response = resp;
    };

    // Act
    authController.registerUser(req, callback);

    // Assert
    ASSERT_TRUE(callbackCalled);
    ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k201Created);
    auto responseBody = response->getJsonObject();
    ASSERT_TRUE(responseBody);
    ASSERT_TRUE(responseBody->isMember("username"));
    ASSERT_TRUE(responseBody->isMember("token"));
}

// Test for loginUser: missing fields
TEST_F(AuthControllerTest, LoginUser_MissingFields) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    auto req = createMockRequest(json);

    bool callbackCalled = false;
    drogon::HttpResponsePtr response;

    auto callback = [&](const drogon::HttpResponsePtr &resp) {
        callbackCalled = true;
        response = resp;
    };

    // Act
    authController.loginUser(req, callback);

    // Assert
    ASSERT_TRUE(callbackCalled);
    ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k400BadRequest);
    auto responseBody = response->getJsonObject();
    ASSERT_TRUE(responseBody);
    ASSERT_EQ((*responseBody)["error"].asString(), "missing fields");
}

// Test for loginUser: user not found
TEST_F(AuthControllerTest, LoginUser_UserNotFound) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    json["password"] = "password123";
    auto req = createMockRequest(json);

    EXPECT_CALL(*mockMapper, findFutureBy(::testing::_))
        .WillOnce(::testing::Return(std::async(std::launch::deferred, []() {
            return std::vector<User>{};
        })));

    bool callbackCalled = false;
    drogon::HttpResponsePtr response;

    auto callback = [&](const drogon::HttpResponsePtr &resp) {
        callbackCalled = true;
        response = resp;
    };

    // Act
    authController.loginUser(req, callback);

    // Assert
    ASSERT_TRUE(callbackCalled);
    ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k400BadRequest);
    auto responseBody = response->getJsonObject();
    ASSERT_TRUE(responseBody);
    ASSERT_EQ((*responseBody)["error"].asString(), "user not found");
}

// Test for loginUser: invalid password
TEST_F(AuthControllerTest, LoginUser_InvalidPassword) {
    // Arrange
    Json::Value json;
    json["username"] = "test_user";
    json["password"] = "wrong_password";
    auto req = createMockRequest(json);

    User dbUser;
    dbUser.setPassword("hashed_password");

    EXPECT_CALL(*mockMapper, findFutureBy(::testing::_))
        .WillOnce(::testing::Return(std::async(std::launch::deferred, [dbUser]() {
            return std::vector<User>{dbUser};
        })));

    EXPECT_CALL(authController, isPasswordValid("wrong_password", "hashed_password"))
        .WillOnce(::testing::Return(false));

    bool callbackCalled = false;
    drogon::HttpResponsePtr response;

    auto callback = [&](const drogon::HttpResponsePtr &resp) {
        callbackCalled = true;
        response = resp;
    };

    // Act
    authController.loginUser(req, callback);

    // Assert
    ASSERT_TRUE(callbackCalled);
    ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k401Unauthorized);
    auto responseBody = response->getJsonObject();
    ASSERT_TRUE(responseBody);
    ASSERT_EQ((*responseBody)["error"].asString(), "username and password do not match");
}