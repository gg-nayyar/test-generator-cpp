import fs from "fs";
import path from "path";
import { execSync } from "child_process";
import dotenv from "dotenv";
import { AzureKeyCredential } from "@azure/core-auth";
import ModelClient, { isUnexpected } from "@azure-rest/ai-inference";

dotenv.config();

const endpoint = "https://models.github.ai/inference";
const token = process.env.GITHUB_TOKEN_BuildAndFix;
const model = "openai/gpt-4o";
const TEST_DIR = path.join("tests", "test_src", "src");
const YAML_FIX_PATH = path.join("instructions", "fix_build.yaml");

const client = ModelClient(endpoint, new AzureKeyCredential(token!));

async function fixTestFile(filePath: string, buildLog: string): Promise<void> {
  const fileContent = fs.readFileSync(filePath, "utf-8");
  const fixInstructions = fs.readFileSync(YAML_FIX_PATH, "utf-8");

  const prompt = `# YAML INSTRUCTIONS\n${fixInstructions}\n\n# BUILD LOGS\n${buildLog}\n\n# BROKEN TEST FILE\n${fileContent}`;

  const response = await client.path("/chat/completions").post({
    body: {
      messages: [
        {
          role: "system",
          content:
            "You are a C++ compiler assistant who fixes broken test files using Google Test.",
        },
        { role: "user", content: prompt },
      ],
      temperature: 0.3,
      top_p: 1,
      model: model,
    },
  });

  if (isUnexpected(response)) throw response.body.error;

  const rawFix = response.body.choices?.[0]?.message?.content ?? "";

const fixed = rawFix
  .replace(/```[a-zA-Z]*\n?/g, "")
  .replace(/^.*?(Here is|###|Changes Made).*\n?/gi, "")
  .replace(/`/g, "")
  .replace(/^\s*\/\/.*$/gm, "")
  .trim();

  fs.writeFileSync(filePath, fixed);

  console.log(`🛠️ Fixed and updated: ${filePath}`);
}

async function tryBuildProject(): Promise<{ success: boolean; log: string }> {
   try {
    // Step 1: Recursively find all .cc/.cpp test files
    const sourceFiles: string[] = [];

    const walk = (dir: string) => {
      fs.readdirSync(dir).forEach((file) => {
        const fullPath = path.join(dir, file);
        if (fs.statSync(fullPath).isDirectory()) {
          walk(fullPath);
        } else if (file.endsWith(".cc") || file.endsWith(".cpp")) {
          sourceFiles.push(fullPath);
        }
      });
    };
    walk(TEST_DIR);

    if (sourceFiles.length === 0) {
      throw new Error("No source files found for build.");
    }

    const command = [
      "g++",
      "-std=c++17",
      ...sourceFiles.map((f) => `"${f}"`),
      "-lgtest",
      "-lgtest_main",
      "-pthread",
      "-o test_executable",
    ].join(" ");

    console.log("Running build command:");
    console.log(command);

    const output = execSync(command, { encoding: "utf-8" });
    console.log("Build succeeded.");
    return { success: true, log: output };
  } catch (err: any) {
    console.error("Build failed.");
    return {
      success: false,
      log: err.stderr?.toString() ?? err.toString(),
    };
  }
}

async function buildAndFixLoop() {
  let attempt = 0;
  while (true) {
    attempt++;
    console.log(`\nBuild attempt ${attempt}...`);

    const { success, log } = await tryBuildProject();
    if (success) break;

    const testFiles: string[] = [];

    const walk = (dir: string) => {
      fs.readdirSync(dir).forEach((file) => {
        const fullPath = path.join(dir, file);
        if (fs.statSync(fullPath).isDirectory()) {
          walk(fullPath);
        } else if (fullPath.endsWith(".cc") || fullPath.endsWith(".cpp")) {
          testFiles.push(fullPath);
        }
      });
    };
    walk(TEST_DIR);

    if (testFiles.length === 0) {
      console.error("No test files found.");
      process.exit(1);
    }

    for (const testFile of testFiles) {
      try {
        await fixTestFile(testFile, log);
      } catch (err) {
        console.error(`Failed to fix ${testFile}:`, err);
      }
    }
  }

  console.log("✅ Build fixed successfully.");
}

if (require.main === module) {
  if (!token || !fs.existsSync(YAML_FIX_PATH)) {
    console.error("Missing GITHUB_TOKEN or YAML instruction file.");
    process.exit(1);
  }

  buildAndFixLoop();
}
