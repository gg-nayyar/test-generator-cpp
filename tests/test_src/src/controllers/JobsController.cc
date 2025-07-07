#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <drogon/HttpAppFramework.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include "JobsController.h"
#include "../models/Job.h"
#include "../models/Person.h"

using namespace drogon;
using namespace drogon_model::org_chart;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class MockDbClient : public drogon::orm::DbClient {
public:
    MOCK_METHOD(void, execSqlAsync, (const std::string &, const std::vector<std::shared_ptr<drogon::orm::SqlBinder>> &, std::function<void(const drogon::orm::Result &)> &&, std::function<void(const std::exception_ptr &)> &&), (const, override));
    MOCK_METHOD(drogon::orm::Result, execSqlSync, (const std::string &, const std::vector<std::shared_ptr<drogon::orm::SqlBinder>> &), (const, override));
};

class JobsControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        dbClient = std::make_shared<MockDbClient>();
        drogon::app().setDbClient(dbClient);
    }

    std::shared_ptr<MockDbClient> dbClient;
    JobsController controller;
};

TEST_F(JobsControllerTest, GetJobs_Success) {
    // Arrange
    auto req = HttpRequest::newHttpRequest();
    req->setParameter("offset", "0");
    req->setParameter("limit", "10");
    req->setParameter("sort_field", "id");
    req->setParameter("sort_order", "asc");

    auto callback = [](const HttpResponsePtr &resp) {
        ASSERT_EQ(resp->getStatusCode(), HttpStatusCode::k200OK);
        ASSERT_FALSE(resp->getJsonObject()->empty());
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::vector<std::shared_ptr<drogon::orm::SqlBinder>> &, std::function<void(const drogon::orm::Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            drogon::orm::Result result;
            successCallback(result);
        }));

    // Act
    controller.get(req, callback);
}

TEST_F(JobsControllerTest, GetOneJob_Success) {
    // Arrange
    int jobId = 1;
    auto req = HttpRequest::newHttpRequest();

    auto callback = [](const HttpResponsePtr &resp) {
        ASSERT_EQ(resp->getStatusCode(), HttpStatusCode::k201Created);
        ASSERT_FALSE(resp->getJsonObject()->empty());
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([jobId](const std::string &, const std::vector<std::shared_ptr<drogon::orm::SqlBinder>> &, std::function<void(const drogon::orm::Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            drogon::orm::Result result;
            successCallback(result);
        }));

    // Act
    controller.getOne(req, callback, jobId);
}

TEST_F(JobsControllerTest, CreateJob_Success) {
    // Arrange
    auto req = HttpRequest::newHttpRequest();
    Job job;
    job.setTitle("New Job");

    auto callback = [](const HttpResponsePtr &resp) {
        ASSERT_EQ(resp->getStatusCode(), HttpStatusCode::k201Created);
        ASSERT_FALSE(resp->getJsonObject()->empty());
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::vector<std::shared_ptr<drogon::orm::SqlBinder>> &, std::function<void(const drogon::orm::Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            drogon::orm::Result result;
            successCallback(result);
        }));

    // Act
    controller.createOne(req, callback, std::move(job));
}

TEST_F(JobsControllerTest, UpdateJob_Success) {
    // Arrange
    int jobId = 1;
    auto req = HttpRequest::newHttpRequest();
    Job jobDetails;
    jobDetails.setTitle("Updated Job");

    auto callback = [](const HttpResponsePtr &resp) {
        ASSERT_EQ(resp->getStatusCode(), HttpStatusCode::k204NoContent);
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::vector<std::shared_ptr<drogon::orm::SqlBinder>> &, std::function<void(const drogon::orm::Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            drogon::orm::Result result;
            successCallback(result);
        }));

    // Act
    controller.updateOne(req, callback, jobId, std::move(jobDetails));
}

TEST_F(JobsControllerTest, DeleteJob_Success) {
    // Arrange
    int jobId = 1;
    auto req = HttpRequest::newHttpRequest();

    auto callback = [](const HttpResponsePtr &resp) {
        ASSERT_EQ(resp->getStatusCode(), HttpStatusCode::k204NoContent);
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::vector<std::shared_ptr<drogon::orm::SqlBinder>> &, std::function<void(const drogon::orm::Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            drogon::orm::Result result;
            successCallback(result);
        }));

    // Act
    controller.deleteOne(req, callback, jobId);
}

TEST_F(JobsControllerTest, GetJobPersons_Success) {
    // Arrange
    int jobId = 1;
    auto req = HttpRequest::newHttpRequest();

    auto callback = [](const HttpResponsePtr &resp) {
        ASSERT_EQ(resp->getStatusCode(), HttpStatusCode::k200OK);
        ASSERT_FALSE(resp->getJsonObject()->empty());
    };

    EXPECT_CALL(*dbClient, execSqlAsync(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::vector<std::shared_ptr<drogon::orm::SqlBinder>> &, std::function<void(const drogon::orm::Result &)> &&successCallback, std::function<void(const std::exception_ptr &)> &&) {
            drogon::orm::Result result;
            successCallback(result);
        }));

    // Act
    controller.getJobPersons(req, callback, jobId);
}