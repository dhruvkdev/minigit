import { exec } from "child_process";
import { promisify } from "util";
import path from "path";
import fs from "fs";
import { NextResponse } from "next/server";

const execAsync = promisify(exec);

const REPO_PATH = path.resolve(process.cwd(), "..");
const MGIT_EXE = path.join(REPO_PATH, "mgit.exe");

interface StatusResult {
  currentBranch: string;
  latestCommit: string;
  latestCommitDate: string;
  latestCommitTime: string;
  stagedFiles: string[];
  branches: string[];
}

function parseStatusOutput(raw: string): Partial<StatusResult> {
  const lines = raw.split("\n").map(l => l.trim()).filter(Boolean);
  const result: Partial<StatusResult> = {
    currentBranch: "main",
    latestCommit: "",
    latestCommitDate: "",
    latestCommitTime: "",
    stagedFiles: [],
  };

  let inStagedSection = false;
  for (const line of lines) {
    if (line.startsWith("Current Branch :-")) {
      result.currentBranch = line.replace("Current Branch :-", "").trim();
    } else if (line.startsWith("Latest Commit :-")) {
      result.latestCommit = line.replace("Latest Commit :-", "").trim();
    } else if (line.startsWith("Date :")) {
      result.latestCommitDate = line.replace("Date :", "").trim();
    } else if (line.startsWith("Time :")) {
      result.latestCommitTime = line.replace("Time :", "").trim();
    } else if (line.startsWith("Staged Files :-")) {
      inStagedSection = true;
    } else if (inStagedSection && line) {
      result.stagedFiles = result.stagedFiles || [];
      result.stagedFiles.push(line);
    }
  }
  return result;
}

function listBranches(): string[] {
  const branchesDir = path.join(REPO_PATH, ".mgit", "refs", "heads");
  if (!fs.existsSync(branchesDir)) return [];
  return fs.readdirSync(branchesDir).filter(f => !f.includes("."));
}

export async function GET() {
  try {
    const { stdout } = await execAsync(`"${MGIT_EXE}" status`, { cwd: REPO_PATH });
    const parsed = parseStatusOutput(stdout);
    const branches = listBranches();
    return NextResponse.json({ ...parsed, branches });
  } catch (error: unknown) {
    const branches = listBranches();
    return NextResponse.json({
      currentBranch: "main",
      latestCommit: "",
      latestCommitDate: "",
      latestCommitTime: "",
      stagedFiles: [],
      branches,
      error: (error as Error).message,
    });
  }
}
