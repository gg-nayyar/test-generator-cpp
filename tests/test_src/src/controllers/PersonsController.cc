#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>
#include <drogon/orm/Result.h>
#include <drogon/HttpTypes.h>
#include <json/json.h>
#include "PersonsController.h"

// Mock classes for dependencies
class MockDbClient : public drogon::orm::DbClient {
public:
    MOCK_METHOD(void, execSqlAsync, (const std::string &, const std::vector<std::string> &, const drogon::orm::ResultCallback &, const drogon::orm::ExceptionCallback &), (const, override));
};

class MockHttpRequest : public drogon::HttpRequest {
public:
    MOCK_METHOD(std::optional<std::string>, getOptionalParameter, (const std::string &), (const, override));
    MOCK_METHOD(std::shared_ptr<Json::Value>, getJsonObject, (), (const, override));
};

class MockHttpResponse : public drogon::HttpResponse {
public:
    MOCK_METHOD(void, setStatusCode, (drogon::HttpStatusCode), (override));
    MOCK_METHOD(void, setBody, (const std::string &), (override));
};

// Test fixture for PersonsController
class PersonsControllerTest : public ::testing::Test {
protected:
    PersonsController controller;
    std::shared_ptr<MockDbClient> mockDbClient;
    std::shared_ptr<MockHttpRequest> mockRequest;
    std::shared_ptr<MockHttpResponse> mockResponse;

    void SetUp() override {
        mockDbClient = std::make_shared<MockDbClient>();
        mockRequest = std::make_shared<MockHttpRequest>();
        mockResponse = std::make_shared<MockHttpResponse>();
    }
};

// Test for PersonsController::get
TEST_F(PersonsControllerTest, GetPersons_Success) {
    // Arrange
    EXPECT_CALL(*mockRequest, getOptionalParameter("sort_field")).WillOnce(::testing::Return("id"));
    EXPECT_CALL(*mockRequest, getOptionalParameter("sort_order")).WillOnce(::testing::Return("asc"));
    EXPECT_CALL(*mockRequest, getOptionalParameter("limit")).WillOnce(::testing::Return("10"));
    EXPECT_CALL(*mockRequest, getOptionalParameter("offset")).WillOnce(::testing::Return("0"));

    drogon::orm::Result mockResult;
    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([&mockResult](const std::string &, const std::vector<std::string> &, const drogon::orm::ResultCallback &resultCallback, const drogon::orm::ExceptionCallback &) {
            resultCallback(mockResult);
        });

    // Act
    controller.get(mockRequest, [](const drogon::HttpResponsePtr &resp) {
        // Assert
        EXPECT_EQ(resp->getStatusCode(), drogon::HttpStatusCode::k200OK);
    });
}

// Test for PersonsController::getOne
TEST_F(PersonsControllerTest, GetOnePerson_NotFound) {
    // Arrange
    int personId = 1;

    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const std::string &, const std::vector<std::string> &, const drogon::orm::ResultCallback &, const drogon::orm::ExceptionCallback &exceptionCallback) {
            exceptionCallback(drogon::orm::DrogonDbException("resource not found"));
        });

    // Act
    controller.getOne(mockRequest, [](const drogon::HttpResponsePtr &resp) {
        // Assert
        EXPECT_EQ(resp->getStatusCode(), drogon::HttpStatusCode::k404NotFound);
    }, personId);
}

// Test for PersonsController::createOne
TEST_F(PersonsControllerTest, CreateOnePerson_Success) {
    // Arrange
    Person mockPerson;
    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const std::string &, const std::vector<std::string> &, const drogon::orm::ResultCallback &resultCallback, const drogon::orm::ExceptionCallback &) {
            drogon::orm::Result mockResult;
            resultCallback(mockResult);
        });

    // Act
    controller.createOne(mockRequest, [](const drogon::HttpResponsePtr &resp) {
        // Assert
        EXPECT_EQ(resp->getStatusCode(), drogon::HttpStatusCode::k201Created);
    }, std::move(mockPerson));
}

// Test for PersonsController::updateOne
TEST_F(PersonsControllerTest, UpdateOnePerson_NotFound) {
    // Arrange
    int personId = 1;
    Person mockPerson;

    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const std::string &, const std::vector<std::string> &, const drogon::orm::ResultCallback &, const drogon::orm::ExceptionCallback &exceptionCallback) {
            exceptionCallback(drogon::orm::DrogonDbException("resource not found"));
        });

    // Act
    controller.updateOne(mockRequest, [](const drogon::HttpResponsePtr &resp) {
        // Assert
        EXPECT_EQ(resp->getStatusCode(), drogon::HttpStatusCode::k404NotFound);
    }, personId, std::move(mockPerson));
}

// Test for PersonsController::deleteOne
TEST_F(PersonsControllerTest, DeleteOnePerson_Success) {
    // Arrange
    int personId = 1;

    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const std::string &, const std::vector<std::string> &, const drogon::orm::ResultCallback &resultCallback, const drogon::orm::ExceptionCallback &) {
            drogon::orm::Result mockResult;
            resultCallback(mockResult);
        });

    // Act
    controller.deleteOne(mockRequest, [](const drogon::HttpResponsePtr &resp) {
        // Assert
        EXPECT_EQ(resp->getStatusCode(), drogon::HttpStatusCode::k204NoContent);
    }, personId);
}

// Test for PersonsController::getDirectReports
TEST_F(PersonsControllerTest, GetDirectReports_NotFound) {
    // Arrange
    int personId = 1;

    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce([](const std::string &, const std::vector<std::string> &, const drogon::orm::ResultCallback &, const drogon::orm::ExceptionCallback &exceptionCallback) {
            exceptionCallback(drogon::orm::DrogonDbException("resource not found"));
        });

    // Act
    controller.getDirectReports(mockRequest, [](const drogon::HttpResponsePtr &resp) {
        // Assert
        EXPECT_EQ(resp->getStatusCode(), drogon::HttpStatusCode::k404NotFound);
    }, personId);
}