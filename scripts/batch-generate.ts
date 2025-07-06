import fs from "fs";
import path from "path";
import dotenv from "dotenv";
import { generateTest } from "./generate";

dotenv.config();

function findCppFiles(dir: string): string[]{
    const results: string[] = [];
    
    const list = fs.readdirSync(dir);
    list.forEach((file) => {
        const fullPath = path.join(dir, file);
        const stat = fs.statSync(fullPath);
        if(stat.isDirectory()){
            results.push(...findCppFiles(fullPath));
        }
        else if(file.endsWith(".cpp") || file.endsWith(".cc")){
            results.push(fullPath);
        }
    });
    return results;
}

async function runBatch() {
  const cppFiles = findCppFiles("src");
  console.log(`🧠 Found ${cppFiles.length} C++ files`);

  for (const fullPath of cppFiles) {
    const relPath = path.relative(process.cwd(), fullPath);
    const fileName = path.basename(fullPath);
    console.log(`📄 Generating test for ${fileName}...`);

    try {
      await generateTest(relPath, "init_test.yaml");
    } catch (err) {
      console.error(`❌ Failed to generate test for ${relPath}:`, err);
    }
  }
  console.log("Batch generation complete!");
}
runBatch()