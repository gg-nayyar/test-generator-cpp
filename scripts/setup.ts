import { execSync } from "child_process";
import fs from "fs";
import path from "path";

const THIRD_PARTY_DIR = path.resolve("src", "third_party");

const repos = [
  {
    name: "drogon",
    url: "https://github.com/drogonframework/drogon.git",
  },
  {
    name: "jwt-cpp",
    url: "https://github.com/Thalhammer/jwt-cpp.git",
  },
  {
    name: "libbcrypt",
    url: "https://github.com/rg3/libbcrypt.git",
  },
  {
    name: "jsoncpp",
    url: "https://github.com/open-source-parsers/jsoncpp.git",
  },
];

function cloneOrSkip({ name, url }: { name: string; url: string }) {
  const dest = path.join(THIRD_PARTY_DIR, name);
  if (fs.existsSync(dest) && fs.readdirSync(dest).length > 0) {
    console.log(`✅ ${name} already exists.`);
    return;
  }
  console.log(`📦 Cloning ${name}...`);
  execSync(`git clone --depth=1 ${url} "${dest}"`, { stdio: "inherit" });
}

function main() {
  if (!fs.existsSync(THIRD_PARTY_DIR)) {
    fs.mkdirSync(THIRD_PARTY_DIR, { recursive: true });
  }

  repos.forEach(cloneOrSkip);
  console.log("✅ All third-party dependencies set up.");
}

main();
