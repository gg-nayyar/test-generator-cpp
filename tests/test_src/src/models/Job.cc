#include <gtest/gtest.h>
#include <json/json.h>
#include <memory>
#include <string>
#include <vector>
#include "Job.h"
#include "Person.h"

// Mock dependencies
namespace drogon {
namespace orm {
class Row {
public:
    bool isNull() const { return false; }
    template <typename T>
    T as() const { return T(); }
    size_t size() const { return 2; }
    const Row& operator[](size_t) const { return *this; }
};
}  // namespace orm
}  // namespace drogon

// Test suite for the Job class
class JobTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test getColumnName
TEST_F(JobTest, GetColumnName_ValidIndex) {
    // Arrange
    size_t index = 0;

    // Act
    const std::string& columnName = Job::getColumnName(index);

    // Assert
    EXPECT_EQ(columnName, "id");
}

TEST_F(JobTest, GetColumnName_InvalidIndex) {
    // Arrange
    size_t index = 10;

    // Act & Assert
    EXPECT_THROW(Job::getColumnName(index), std::exception);
}

// Test constructors
TEST_F(JobTest, Constructor_FromRow) {
    // Arrange
    drogon::orm::Row row;

    // Act
    Job job(row);

    // Assert
    EXPECT_EQ(job.getValueOfId(), 0);
    EXPECT_EQ(job.getValueOfTitle(), "");
}

TEST_F(JobTest, Constructor_FromJson) {
    // Arrange
    Json::Value json;
    json["id"] = 123;
    json["title"] = "Software Engineer";

    // Act
    Job job(json);

    // Assert
    EXPECT_EQ(job.getValueOfId(), 123);
    EXPECT_EQ(job.getValueOfTitle(), "Software Engineer");
}

// Test setters and getters
TEST_F(JobTest, SetAndGetId) {
    // Arrange
    Job job;

    // Act
    job.setId(42);

    // Assert
    EXPECT_EQ(job.getValueOfId(), 42);
}

TEST_F(JobTest, SetAndGetTitle) {
    // Arrange
    Job job;

    // Act
    job.setTitle("Manager");

    // Assert
    EXPECT_EQ(job.getValueOfTitle(), "Manager");
}

// Test toJson
TEST_F(JobTest, ToJson) {
    // Arrange
    Job job;
    job.setId(1);
    job.setTitle("Developer");

    // Act
    Json::Value json = job.toJson();

    // Assert
    EXPECT_EQ(json["id"].asInt(), 1);
    EXPECT_EQ(json["title"].asString(), "Developer");
}

// Test validateJsonForCreation
TEST_F(JobTest, ValidateJsonForCreation_ValidJson) {
    // Arrange
    Json::Value json;
    json["title"] = "Engineer";
    std::string error;

    // Act
    bool isValid = Job::validateJsonForCreation(json, error);

    // Assert
    EXPECT_TRUE(isValid);
    EXPECT_TRUE(error.empty());
}

TEST_F(JobTest, ValidateJsonForCreation_InvalidJson) {
    // Arrange
    Json::Value json;
    std::string error;

    // Act
    bool isValid = Job::validateJsonForCreation(json, error);

    // Assert
    EXPECT_FALSE(isValid);
    EXPECT_EQ(error, "The title column cannot be null");
}

// Test validateJsonForUpdate
TEST_F(JobTest, ValidateJsonForUpdate_ValidJson) {
    // Arrange
    Json::Value json;
    json["id"] = 1;
    json["title"] = "Engineer";
    std::string error;

    // Act
    bool isValid = Job::validateJsonForUpdate(json, error);

    // Assert
    EXPECT_TRUE(isValid);
    EXPECT_TRUE(error.empty());
}

TEST_F(JobTest, ValidateJsonForUpdate_InvalidJson) {
    // Arrange
    Json::Value json;
    json["title"] = "Engineer";
    std::string error;

    // Act
    bool isValid = Job::validateJsonForUpdate(json, error);

    // Assert
    EXPECT_FALSE(isValid);
    EXPECT_EQ(error, "The value of primary key must be set in the json object for update");
}

// Test updateByJson
TEST_F(JobTest, UpdateByJson) {
    // Arrange
    Job job;
    Json::Value json;
    json["id"] = 2;
    json["title"] = "Architect";

    // Act
    job.updateByJson(json);

    // Assert
    EXPECT_EQ(job.getValueOfId(), 2);
    EXPECT_EQ(job.getValueOfTitle(), "Architect");
}

// Test updateByMasqueradedJson
TEST_F(JobTest, UpdateByMasqueradedJson) {
    // Arrange
    Job job;
    Json::Value json;
    json["custom_id"] = 3;
    json["custom_title"] = "Designer";
    std::vector<std::string> masqueradingVector = {"custom_id", "custom_title"};

    // Act
    job.updateByMasqueradedJson(json, masqueradingVector);

    // Assert
    EXPECT_EQ(job.getValueOfId(), 3);
    EXPECT_EQ(job.getValueOfTitle(), "Designer");
}

// Test getPersons
TEST_F(JobTest, GetPersons) {
    // Arrange
    Job job;
    job.setId(1);
    auto mockClientPtr = std::make_shared<drogon::orm::DbClient>();
    std::vector<Person> persons;

    // Act & Assert
    EXPECT_NO_THROW(job.getPersons(
        mockClientPtr,
        [&](std::vector<Person> result) { persons = result; },
        [](const std::exception& e) { FAIL() << e.what(); }));
}