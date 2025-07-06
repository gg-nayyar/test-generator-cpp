import fs from "fs";
import yaml from "yaml";
import path from "path";
import dotenv from "dotenv";
import { AzureKeyCredential } from "@azure/core-auth";
import ModelClient, { isUnexpected } from "@azure-rest/ai-inference";

dotenv.config();
const token = process.env.GITHUB_TOKEN;
const endpoint = "https://models.github.ai/inference";
const model = "openai/gpt-4o";

if (!token) {
  console.error("GITHUB_TOKEN is missing in your .env file.");
  process.exit(1);
}

const client = ModelClient(endpoint, new AzureKeyCredential(token!));

function writeFileSafely(filePath: string, content: string) {
  const dir = path.dirname(filePath);
  fs.mkdirSync(dir, { recursive: true });
  fs.writeFileSync(filePath, content);
  console.log(`✅ Test written to: ${filePath}`);
}

export async function generateTest(
  cppFile: string,
  yamlFile: string
): Promise<void> {
  try {
    const cppPath = path.join(cppFile);
    const yamlPath = path.join("instructions", yamlFile);
    if (!fs.existsSync(yamlPath)) {
      throw new Error(`YAML file not found: ${yamlPath}`);
    }
    if (!fs.existsSync(cppPath)) {
      throw new Error(`C++ file not found: ${cppPath}`);
    }
    const source = fs.readFileSync(cppPath, "utf-8");
    const promptYaml = fs.readFileSync(
      path.join("instructions", yamlFile),
      "utf-8"
    );
    const cleanedSource = source
      .split("\n")
      .map((line) => {
        if (line.includes("<third_party/"))
          return `// ${line}  // skipped unknown include`;
        return line;
      })
      .join("\n");

    console.log(`🛠️ Reading C++ file: ${cppFile}`);
    console.log(`📂 Full path: ${cppPath}`);
    console.log("process.argv:", process.argv);

    const prompt = `# YAML INSTRUCTIONS\n${promptYaml}\n\n# SOURCE CODE\n${cleanedSource}`;

    const response = await client.path("/chat/completions").post({
      body: {
        messages: [
          {
            role: "system",
            content: " You are a C++ test generator using Google Test.",
          },
          { role: "user", content: prompt },
        ],
        temperature: 0.3,
        top_p: 1,
        model: model,
      },
    });
    console.log("RAW MODEL RESPONSE: ", response)
    if (isUnexpected(response)) throw response.body.error;
     const testRelativePath = path.join(
      "tests",
      "test_src",
      cppFile.replace(/\.cpp$/, ".cc")
    );

    const testContent = response.body.choices?.[0]?.message?.content ?? "";

    writeFileSafely(testRelativePath, testContent);
  } catch (error: any) {
    console.error("Error generating test:", error);
    if (typeof error === "string" || typeof error?.message === "string") {
      console.error(error.message ?? error);
    } else {
      console.error(JSON.stringify(error, null, 2));
    }
    process.exit(1);
  }
}
if (require.main === module) {
  const [cppFile, yamlFile] = process.argv.slice(2);
  if (!cppFile || !yamlFile) {
    console.error("Usage: ts-node generate.ts <cppFile> <yamlFile>");
    process.exit(1);
  }
  generateTest(cppFile, yamlFile);
}
