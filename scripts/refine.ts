import fs from "fs";
import path from "path";
import dotenv from "dotenv";
import { AzureKeyCredential } from "@azure/core-auth";
import ModelClient, { isUnexpected } from "@azure-rest/ai-inference";

dotenv.config();

const token = process.env.GITHUB_TOKEN_Refine;
const endpoint = "https://models.github.ai/inference";
const model = "openai/gpt-4o";
const TEST_DIR = path.join("tests", "test_src", "src");
const YAML_PATH = path.resolve("instructions", "refine_test.yaml");

if (!token) {
  console.error("GITHUB_TOKEN missing from .env");
  process.exit(1);
}
if (!fs.existsSync(YAML_PATH)) {
  console.error("refine_tests.yaml file not found.");
  process.exit(1);
}

const client = ModelClient(endpoint, new AzureKeyCredential(token));
const yamlContent = fs.readFileSync(YAML_PATH, "utf-8");

async function refineTestFile(filePath: string) {
  const content = fs.readFileSync(filePath, "utf-8");

  const prompt = `# YAML INSTRUCTIONS\n${yamlContent}\n\n# UNIT TEST FILE\n${content}`;

  const response = await client.path("/chat/completions").post({
    body: {
      messages: [
        {
          role: "system",
          content: "You are a C++ test refiner using Google Test.",
        },
        { role: "user", content: prompt },
      ],
      temperature: 0.2,
      top_p: 1,
      model: model,
    },
  });

  if (isUnexpected(response)) throw response.body.error;

  const refined = response.body.choices?.[0]?.message?.content ?? "";
  fs.writeFileSync(filePath, refined);
  console.log(`Refined: ${filePath}`);
}

async function processTestsInBatch() {
  const files: string[] = [];

  const walk = (dir: string) => {
    fs.readdirSync(dir).forEach((file) => {
      const fullPath = path.join(dir, file);
      if (fs.statSync(fullPath).isDirectory()) {
        walk(fullPath);
      } else if (fullPath.endsWith(".cc") || fullPath.endsWith(".cpp")) {
        files.push(fullPath);
      }
    });
  };

  walk(TEST_DIR);

  if (files.length === 0) {
    console.error("No test files found in:", TEST_DIR);
    return;
  }

  for (const file of files) {
    try {
      await refineTestFile(file);
    } catch (error) {
      console.error(`Error refining ${file}:`, error);
    }
  }

  console.log("Batch refinement complete.");
}

if (require.main === module) {
  processTestsInBatch();
}
