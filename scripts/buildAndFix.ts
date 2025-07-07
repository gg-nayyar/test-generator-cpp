import fs from "fs";
import path from "path";
import { execSync } from "child_process";
import { sendToLLMAndFix } from "./llmHelper"; // Your LLM integration

const SRC_DIR = path.resolve("src");
const TEST_SRC_DIR = path.resolve("tests", "test_src", "src");
const TEST_DIR = path.resolve("tests");
const BUILD_DIR = path.resolve(TEST_DIR, "build");
const CMAKE_FILE = path.join(TEST_DIR, "CMakeLists.txt");
const BUILD_LOG = path.join(BUILD_DIR, "build.log");

function posixify(p: string): string {
  return p.replace(/\\/g, "/");
}

function generateCMakeLists() {
  const srcPath = posixify(SRC_DIR);
  const testPath = posixify(TEST_SRC_DIR);
  const buildPath = posixify(BUILD_DIR);

  const cmake = `
cmake_minimum_required(VERSION 3.14)
project(cpp_test_runner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${buildPath}")

enable_testing()

include_directories("${srcPath}")

file(GLOB_RECURSE SRC_FILES "${srcPath}/*.cc")
file(GLOB_RECURSE TEST_SOURCES "${testPath}/*.cc")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(tests \${SRC_FILES} \${TEST_SOURCES})
target_link_libraries(tests gtest_main)
add_test(NAME AllTests COMMAND tests)
`;

  fs.writeFileSync(CMAKE_FILE, cmake);
  console.log(`📄 CMakeLists.txt written to ${CMAKE_FILE}`);
}

function configureAndBuild(): boolean {
  if (!fs.existsSync(BUILD_DIR)) fs.mkdirSync(BUILD_DIR, { recursive: true });

  try {
    console.log("🛠️  Configuring with CMake...");
    execSync(`cmake ..`, { cwd: BUILD_DIR, stdio: "inherit" });

    console.log("🔨 Building project...");
    execSync(`cmake --build .`, { cwd: BUILD_DIR, stdio: "inherit" });

    return true;
  } catch (err) {
    console.error("❌ Build failed.");
    return false;
  }
}

function getFirstFailingTestFile(): string | null {
  const allFiles: string[] = [];

  function walk(dir: string) {
    const entries = fs.readdirSync(dir, { withFileTypes: true });
    for (const entry of entries) {
      const fullPath = path.join(dir, entry.name);
      if (entry.isDirectory()) {
        walk(fullPath);
      } else if (entry.name.endsWith(".cc")) {
        allFiles.push(fullPath);
      }
    }
  }

  walk(TEST_SRC_DIR);
  return allFiles.length ? allFiles[0] : null;
}

async function main() {
  generateCMakeLists();

  let attempts = 0;
  const maxAttempts = 3;

  while (attempts < maxAttempts) {
    console.log(`🚀 Attempt ${attempts + 1}...`);
    const success = configureAndBuild();

    if (success) {
      console.log("✅ Build succeeded.");
      return;
    }

    const failingFile = getFirstFailingTestFile();
    if (!failingFile || !fs.existsSync(failingFile)) {
      console.error("❌ No failing test file found.");
      return;
    }

    const logs = fs.existsSync(BUILD_LOG)
      ? fs.readFileSync(BUILD_LOG, "utf-8")
      : "Build logs not found or empty.";

    console.log("🤖 Sending logs to LLM...");
    const updatedContent = await sendToLLMAndFix(failingFile, logs);

    fs.writeFileSync(failingFile, updatedContent);
    console.log(`🔧 Fix applied to: ${failingFile}`);

    attempts++;
  }

  console.log("❌ All attempts failed. Please check manually.");
}

main();
