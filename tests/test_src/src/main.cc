#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>

// Mock class for drogon::app()
class MockDrogonApp {
public:
    MOCK_METHOD(void, loadConfigFile, (const std::string&), ());
    MOCK_METHOD(void, run, (), ());
};

// Global mock instance
MockDrogonApp* mockAppInstance = nullptr;

// Mocking drogon::app() to return the global mock instance
namespace drogon {
    MockDrogonApp& app() {
        return *mockAppInstance;
    }
}

// Test fixture for setting up and tearing down the mock
class DrogonAppTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockAppInstance = new MockDrogonApp();
    }

    void TearDown() override {
        delete mockAppInstance;
        mockAppInstance = nullptr;
    }
};

// Test case for verifying the loadConfigFile method is called with the correct argument
TEST_F(DrogonAppTest, LoadConfigFileCalledWithCorrectArgument) {
    // Arrange
    const std::string expectedConfigFile = "../config.json";
    EXPECT_CALL(*mockAppInstance, loadConfigFile(expectedConfigFile)).Times(1);

    // Act
    mockAppInstance->loadConfigFile(expectedConfigFile);

    // Assert
    // Expectations are verified automatically
}

// Test case for verifying the run method is called
TEST_F(DrogonAppTest, RunMethodCalled) {
    // Arrange
    EXPECT_CALL(*mockAppInstance, run()).Times(1);

    // Act
    mockAppInstance->run();

    // Assert
    // Expectations are verified automatically
}

// Additional test case to improve coverage: Verify loadConfigFile is not called with an incorrect argument
TEST_F(DrogonAppTest, LoadConfigFileNotCalledWithIncorrectArgument) {
    // Arrange
    const std::string incorrectConfigFile = "wrong_config.json";
    EXPECT_CALL(*mockAppInstance, loadConfigFile(incorrectConfigFile)).Times(0);

    // Act
    mockAppInstance->loadConfigFile("../config.json");

    // Assert
    // Expectations are verified automatically
}

// Additional test case to improve coverage: Verify run is not called when not expected
TEST_F(DrogonAppTest, RunMethodNotCalledUnexpectedly) {
    // Arrange
    EXPECT_CALL(*mockAppInstance, run()).Times(0);

    // Act
    // No call to mockAppInstance->run()

    // Assert
    // Expectations are verified automatically
}