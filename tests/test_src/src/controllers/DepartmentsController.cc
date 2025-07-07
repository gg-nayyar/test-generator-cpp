#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <drogon/HttpAppFramework.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/orm/Mapper.h>
#include <drogon/orm/Criteria.h>
#include <json/json.h>
#include <memory>
#include "DepartmentsController.h"
#include "../models/Department.h"
#include "../models/Person.h"

using namespace drogon;
using namespace drogon::orm;
using namespace drogon_model::org_chart;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class MockDbClient : public DbClient {
public:
    MOCK_METHOD(void, execSqlAsync, (const std::string &, const std::vector<std::shared_ptr<drogon::orm::SqlBinder>> &, std::function<void(const Result &)> &&, std::function<void(const std::exception_ptr &)> &&), (const, override));
    MOCK_METHOD(Result, execSqlSync, (const std::string &, const std::vector<std::shared_ptr<drogon::orm::SqlBinder>> &), (const, override));
};

class DepartmentsControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        dbClient = std::make_shared<MockDbClient>();
        controller = std::make_unique<DepartmentsController>();
    }

    std::shared_ptr<MockDbClient> dbClient;
    std::unique_ptr<DepartmentsController> controller;
};

TEST_F(DepartmentsControllerTest, GetDepartments_Success) {
    // Arrange
    auto req = HttpRequest::newHttpRequest();
    req->setParameter("offset", "0");
    req->setParameter("limit", "10");
    req->setParameter("sort_field", "name");
    req->setParameter("sort_order", "asc");

    auto callback = [](const HttpResponsePtr &response) {
        ASSERT_EQ(response->getStatusCode(), HttpStatusCode::k200OK);
        auto jsonBody = response->getJsonObject();
        ASSERT_TRUE(jsonBody != nullptr);
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::vector<std::shared_ptr<SqlBinder>> &, std::function<void(const Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            Result result;
            successCallback(result);
        }));

    // Act
    controller->get(req, callback);
}

TEST_F(DepartmentsControllerTest, GetOneDepartment_Success) {
    // Arrange
    int departmentId = 1;
    auto req = HttpRequest::newHttpRequest();

    auto callback = [](const HttpResponsePtr &response) {
        ASSERT_EQ(response->getStatusCode(), HttpStatusCode::k201Created);
        auto jsonBody = response->getJsonObject();
        ASSERT_TRUE(jsonBody != nullptr);
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([departmentId](const std::string &, const std::vector<std::shared_ptr<SqlBinder>> &, std::function<void(const Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            Result result;
            successCallback(result);
        }));

    // Act
    controller->getOne(req, callback, departmentId);
}

TEST_F(DepartmentsControllerTest, CreateOneDepartment_Success) {
    // Arrange
    auto req = HttpRequest::newHttpJsonRequest(Json::Value());
    Department department;
    auto callback = [](const HttpResponsePtr &response) {
        ASSERT_EQ(response->getStatusCode(), HttpStatusCode::k201Created);
        auto jsonBody = response->getJsonObject();
        ASSERT_TRUE(jsonBody != nullptr);
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::vector<std::shared_ptr<SqlBinder>> &, std::function<void(const Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            Result result;
            successCallback(result);
        }));

    // Act
    controller->createOne(req, callback, std::move(department));
}

TEST_F(DepartmentsControllerTest, UpdateOneDepartment_Success) {
    // Arrange
    int departmentId = 1;
    auto req = HttpRequest::newHttpJsonRequest(Json::Value());
    Department department;
    auto callback = [](const HttpResponsePtr &response) {
        ASSERT_EQ(response->getStatusCode(), HttpStatusCode::k204NoContent);
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::vector<std::shared_ptr<SqlBinder>> &, std::function<void(const Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            Result result;
            successCallback(result);
        }));

    // Act
    controller->updateOne(req, callback, departmentId, std::move(department));
}

TEST_F(DepartmentsControllerTest, DeleteOneDepartment_Success) {
    // Arrange
    int departmentId = 1;
    auto req = HttpRequest::newHttpRequest();
    auto callback = [](const HttpResponsePtr &response) {
        ASSERT_EQ(response->getStatusCode(), HttpStatusCode::k204NoContent);
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::vector<std::shared_ptr<SqlBinder>> &, std::function<void(const Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            Result result;
            successCallback(result);
        }));

    // Act
    controller->deleteOne(req, callback, departmentId);
}

TEST_F(DepartmentsControllerTest, GetDepartmentPersons_Success) {
    // Arrange
    int departmentId = 1;
    auto req = HttpRequest::newHttpRequest();
    auto callback = [](const HttpResponsePtr &response) {
        ASSERT_EQ(response->getStatusCode(), HttpStatusCode::k200OK);
        auto jsonBody = response->getJsonObject();
        ASSERT_TRUE(jsonBody != nullptr);
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::vector<std::shared_ptr<SqlBinder>> &, std::function<void(const Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            Result result;
            successCallback(result);
        }));

    // Act
    controller->getDepartmentPersons(req, callback, departmentId);
}