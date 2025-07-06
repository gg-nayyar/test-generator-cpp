import fs from 'fs';
import path from 'path';
import { execSync, spawnSync } from 'child_process';

// Constants
const TEST_DIR = path.resolve('tests');
const TEST_SRC_DIR = path.resolve(TEST_DIR, 'test_src');
const SRC_DIR = path.resolve('src');
const BUILD_DIR = path.resolve(TEST_DIR, 'build');
const EXECUTABLE_NAME = 'test_executable';
const MAX_ATTEMPTS = 3;

// Convert Windows paths to POSIX (important for CMake)
function posixify(p: string): string {
  return p.replace(/\\/g, '/');
}

// Write CMakeLists.txt in /tests
function writeCMakeLists() {
  const srcPath = posixify(SRC_DIR);
  const srcBuildPath = posixify(path.join(BUILD_DIR, 'src_build'));
  
  console.log("🔧 Writing add_subdirectory with paths:");
  console.log("  srcPath:", srcPath);
  console.log("  srcBuildPath:", srcBuildPath);
  const content = `
cmake_minimum_required(VERSION 3.10)
project(${EXECUTABLE_NAME})

enable_testing()

# Add src folder (dev code, readonly)
add_subdirectory(${srcPath} ${srcBuildPath})


# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(GTest REQUIRED)
include_directories(\${GTEST_INCLUDE_DIRS})

file(GLOB_RECURSE TEST_SOURCES "${posixify(TEST_SRC_DIR)}/*.cc")
add_executable(${EXECUTABLE_NAME} \${TEST_SOURCES})
target_link_libraries(${EXECUTABLE_NAME} GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${EXECUTABLE_NAME})
`;
  fs.writeFileSync(path.join(TEST_DIR, 'CMakeLists.txt'), content);
  console.log('📄 CMakeLists.txt written to /tests');
}

// Run CMake & Build
function runBuild(): boolean {
  console.log('🚀 Attempting build...');
  fs.mkdirSync(BUILD_DIR, { recursive: true });

  const cmakeCmd = spawnSync('cmake', ['..'], {
    cwd: BUILD_DIR,
    encoding: 'utf-8',
    shell: true,
  });

  if (cmakeCmd.error) {
    console.error('❌ CMake configuration failed:', cmakeCmd.error.message);
    return false;
  }

  const buildCmd = spawnSync('cmake', ['--build', '.'], {
    cwd: BUILD_DIR,
    encoding: 'utf-8',
    shell: true,
  });

  const logFilePath = path.join(BUILD_DIR, 'build.log');
  fs.writeFileSync(logFilePath, buildCmd.stdout + buildCmd.stderr);

  if (buildCmd.status !== 0) {
    console.log('❌ Build failed.');
    return false;
  }

  console.log('✅ Build succeeded.');
  return true;
}

// Parse logs and identify file to fix
function parseBuildLogsAndFix(): { fileToFix: string; logs: string } | null {
  const logPath = path.join(BUILD_DIR, 'build.log');
  if (!fs.existsSync(logPath)) return null;

  const log = fs.readFileSync(logPath, 'utf-8');
  const match = log.match(/(.*\.cc):\d+:\d+/);
  if (!match) return null;

  const fileToFix = match[1];
  return { fileToFix, logs: log };
}

// Send to LLM for fix
function sendToLLMAndFix(filePath: string, logs: string): void {
  const content = fs.readFileSync(filePath, 'utf-8');

  // Replace this with your LLM API call (e.g., local model or GitHub Copilot)
  console.log(`🤖 Sending logs to LLM...`);
  console.log(`🔧 Applying fix to: ${filePath}`);

  const fixedContent = content; // Replace with LLM response
  fs.writeFileSync(filePath, fixedContent, 'utf-8');
}

// Main script
async function main() {
  writeCMakeLists();

  let success = false;
  for (let attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
    console.log(`🚀 Attempt ${attempt}...`);
    const result = runBuild();
    if (result) {
      success = true;
      break;
    }

    const fixData = parseBuildLogsAndFix();
    if (!fixData) {
      console.log('❌ No actionable errors found.');
      break;
    }

    sendToLLMAndFix(fixData.fileToFix, fixData.logs);
  }

  if (!success) {
    console.log('❌ All attempts failed. Please check manually.');
  }
}

main().catch((err) => console.error(err));
