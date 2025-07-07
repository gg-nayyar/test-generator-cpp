import { execSync } from "child_process";
import path from "path";
import os from "os";

const BUILD_DIR = path.resolve("tests", "build");

// This handles cross-platform test binary name
const testExecutable = os.platform() === "win32"
  ? path.join(BUILD_DIR, "tests.exe")
  : path.join(BUILD_DIR, "tests");

function runTests() {
  console.log("Running unit tests...");
  execSync(testExecutable, {
    stdio: "inherit",
    shell: os.platform() === "win32" ? "cmd.exe" : "/bin/sh", // needed for .exe on Windows
  });
}

function runCoverageTools() {
  console.log("Generating coverage reports...");
  try {
    // Works if gcc/gcov is installed and coverage flags are used
    execSync("gcov *.gcda", {
      cwd: BUILD_DIR,
      stdio: "inherit",
      shell: os.platform() === "win32" ? "cmd.exe" : "/bin/sh",
    });
  } catch (err) {
    console.error("gcov failed:", err);
  }
}

function main() {
  try {
    runTests();
    runCoverageTools();
    console.log("✅ Coverage analysis complete.");
  } catch (err) {
    console.error("❌ Coverage generation failed:", err);
  }
}

main();
