#include <gtest/gtest.h>
#include <json/json.h>
#include <memory>
#include <string>
#include <vector>
#include "Department.h"

// Mocking dependencies
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
class SqlBinder {};
}  // namespace orm
}  // namespace drogon

// Test suite for the Department class
class DepartmentTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test getColumnName method
TEST_F(DepartmentTest, GetColumnName_ValidIndex) {
    // Arrange
    size_t index = 0;

    // Act
    std::string columnName = Department::getColumnName(index);

    // Assert
    EXPECT_EQ(columnName, "id");
}

TEST_F(DepartmentTest, GetColumnName_InvalidIndex) {
    // Arrange
    size_t index = 10;

    // Act & Assert
    EXPECT_THROW(Department::getColumnName(index), std::exception);
}

// Test constructors
TEST_F(DepartmentTest, Constructor_FromRow) {
    // Arrange
    drogon::orm::Row row;

    // Act
    Department department(row, -1);

    // Assert
    EXPECT_EQ(department.getValueOfId(), 0);
    EXPECT_EQ(department.getValueOfName(), "");
}

TEST_F(DepartmentTest, Constructor_FromJson) {
    // Arrange
    Json::Value json;
    json["id"] = 1;
    json["name"] = "HR";

    // Act
    Department department(json);

    // Assert
    EXPECT_EQ(department.getValueOfId(), 1);
    EXPECT_EQ(department.getValueOfName(), "HR");
}

// Test setters and getters
TEST_F(DepartmentTest, SetAndGetId) {
    // Arrange
    Department department;

    // Act
    department.setId(42);

    // Assert
    EXPECT_EQ(department.getValueOfId(), 42);
}

TEST_F(DepartmentTest, SetAndGetName) {
    // Arrange
    Department department;

    // Act
    department.setName("Finance");

    // Assert
    EXPECT_EQ(department.getValueOfName(), "Finance");
}

// Test toJson method
TEST_F(DepartmentTest, ToJson) {
    // Arrange
    Department department;
    department.setId(10);
    department.setName("Engineering");

    // Act
    Json::Value json = department.toJson();

    // Assert
    EXPECT_EQ(json["id"].asInt(), 10);
    EXPECT_EQ(json["name"].asString(), "Engineering");
}

// Test validateJsonForCreation
TEST_F(DepartmentTest, ValidateJsonForCreation_ValidJson) {
    // Arrange
    Json::Value json;
    json["name"] = "Marketing";
    std::string err;

    // Act
    bool isValid = Department::validateJsonForCreation(json, err);

    // Assert
    EXPECT_TRUE(isValid);
    EXPECT_TRUE(err.empty());
}

TEST_F(DepartmentTest, ValidateJsonForCreation_InvalidJson) {
    // Arrange
    Json::Value json;
    std::string err;

    // Act
    bool isValid = Department::validateJsonForCreation(json, err);

    // Assert
    EXPECT_FALSE(isValid);
    EXPECT_EQ(err, "The name column cannot be null");
}

// Test validateJsonForUpdate
TEST_F(DepartmentTest, ValidateJsonForUpdate_ValidJson) {
    // Arrange
    Json::Value json;
    json["id"] = 1;
    json["name"] = "Sales";
    std::string err;

    // Act
    bool isValid = Department::validateJsonForUpdate(json, err);

    // Assert
    EXPECT_TRUE(isValid);
    EXPECT_TRUE(err.empty());
}

TEST_F(DepartmentTest, ValidateJsonForUpdate_InvalidJson) {
    // Arrange
    Json::Value json;
    json["name"] = "Sales";
    std::string err;

    // Act
    bool isValid = Department::validateJsonForUpdate(json, err);

    // Assert
    EXPECT_FALSE(isValid);
    EXPECT_EQ(err, "The value of primary key must be set in the json object for update");
}

// Test validJsonOfField
TEST_F(DepartmentTest, ValidJsonOfField_ValidId) {
    // Arrange
    Json::Value json;
    json = 1;
    std::string err;

    // Act
    bool isValid = Department::validJsonOfField(0, "id", json, err, false);

    // Assert
    EXPECT_TRUE(isValid);
    EXPECT_TRUE(err.empty());
}

TEST_F(DepartmentTest, ValidJsonOfField_InvalidId) {
    // Arrange
    Json::Value json;
    json = "invalid";
    std::string err;

    // Act
    bool isValid = Department::validJsonOfField(0, "id", json, err, false);

    // Assert
    EXPECT_FALSE(isValid);
    EXPECT_EQ(err, "Type error in the id field");
}

TEST_F(DepartmentTest, ValidJsonOfField_ValidName) {
    // Arrange
    Json::Value json;
    json = "Valid Name";
    std::string err;

    // Act
    bool isValid = Department::validJsonOfField(1, "name", json, err, false);

    // Assert
    EXPECT_TRUE(isValid);
    EXPECT_TRUE(err.empty());
}

TEST_F(DepartmentTest, ValidJsonOfField_InvalidName) {
    // Arrange
    Json::Value json;
    json = 12345;
    std::string err;

    // Act
    bool isValid = Department::validJsonOfField(1, "name", json, err, false);

    // Assert
    EXPECT_FALSE(isValid);
    EXPECT_EQ(err, "Type error in the name field");
}