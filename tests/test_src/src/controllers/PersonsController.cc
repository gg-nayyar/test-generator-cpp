#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/orm/Mapper.h>
#include <drogon/orm/Criteria.h>
#include "PersonsController.h"

// Mocks for dependencies
class MockDbClient : public drogon::orm::DbClient {
public:
    MOCK_METHOD(void, execSqlAsync, (const std::string &, const std::vector<std::string> &, drogon::orm::ResultCallback, drogon::orm::ExceptionCallback), (const, override));
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
    EXPECT_CALL(*mockRequest, getOptionalParameter("sort_field"))
        .WillOnce(::testing::Return("id"));
    EXPECT_CALL(*mockRequest, getOptionalParameter("sort_order"))
        .WillOnce(::testing::Return("asc"));
    EXPECT_CALL(*mockRequest, getOptionalParameter("limit"))
        .WillOnce(::testing::Return("25"));
    EXPECT_CALL(*mockRequest, getOptionalParameter("offset"))
        .WillOnce(::testing::Return("0"));

    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke([](const std::string &, const std::vector<std::string> &, drogon::orm::ResultCallback resultCb, drogon::orm::ExceptionCallback) {
            drogon::orm::Result result; // Mock result
            resultCb(result);
        }));

    // Act
    controller.get(mockRequest, [](const drogon::HttpResponsePtr &response) {
        // Assert
        ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k200OK);
    });
}

// Test for PersonsController::getOne
TEST_F(PersonsControllerTest, GetOnePerson_NotFound) {
    // Arrange
    int personId = 1;

    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke([](const std::string &, const std::vector<std::string> &, drogon::orm::ResultCallback resultCb, drogon::orm::ExceptionCallback) {
            drogon::orm::Result result; // Empty result
            resultCb(result);
        }));

    // Act
    controller.getOne(mockRequest, [](const drogon::HttpResponsePtr &response) {
        // Assert
        ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k404NotFound);
    }, personId);
}

// Test for PersonsController::createOne
TEST_F(PersonsControllerTest, CreateOnePerson_Success) {
    // Arrange
    Person person;
    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke([](const std::string &, const std::vector<std::string> &, drogon::orm::ResultCallback resultCb, drogon::orm::ExceptionCallback) {
            drogon::orm::Result result; // Mock result
            resultCb(result);
        }));

    // Act
    controller.createOne(mockRequest, [](const drogon::HttpResponsePtr &response) {
        // Assert
        ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k201Created);
    }, std::move(person));
}

// Test for PersonsController::updateOne
TEST_F(PersonsControllerTest, UpdateOnePerson_NotFound) {
    // Arrange
    int personId = 1;
    Person person;

    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke([](const std::string &, const std::vector<std::string> &, drogon::orm::ResultCallback, drogon::orm::ExceptionCallback exceptionCb) {
            exceptionCb(drogon::orm::DrogonDbException("Not found"));
        }));

    // Act
    controller.updateOne(mockRequest, [](const drogon::HttpResponsePtr &response) {
        // Assert
        ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k404NotFound);
    }, personId, std::move(person));
}

// Test for PersonsController::deleteOne
TEST_F(PersonsControllerTest, DeleteOnePerson_Success) {
    // Arrange
    int personId = 1;

    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke([](const std::string &, const std::vector<std::string> &, drogon::orm::ResultCallback resultCb, drogon::orm::ExceptionCallback) {
            drogon::orm::Result result; // Mock result
            resultCb(result);
        }));

    // Act
    controller.deleteOne(mockRequest, [](const drogon::HttpResponsePtr &response) {
        // Assert
        ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k204NoContent);
    }, personId);
}

// Test for PersonsController::getDirectReports
TEST_F(PersonsControllerTest, GetDirectReports_DatabaseError) {
    // Arrange
    int personId = 1;

    EXPECT_CALL(*mockDbClient, execSqlAsync(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .WillOnce(::testing::Invoke([](const std::string &, const std::vector<std::string> &, drogon::orm::ResultCallback, drogon::orm::ExceptionCallback exceptionCb) {
            exceptionCb(drogon::orm::DrogonDbException("Database error"));
        }));

    // Act
    controller.getDirectReports(mockRequest, [](const drogon::HttpResponsePtr &response) {
        // Assert
        ASSERT_EQ(response->getStatusCode(), drogon::HttpStatusCode::k500InternalServerError);
    }, personId);
}