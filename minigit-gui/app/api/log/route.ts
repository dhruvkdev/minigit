import { exec } from "child_process";
import { promisify } from "util";
import path from "path";
import { NextResponse } from "next/server";

const execAsync = promisify(exec);

// The mgit.exe binary is in the parent folder (OOPs Project root)
const REPO_PATH = path.resolve(process.cwd(), "..");
const MGIT_EXE = path.join(REPO_PATH, "mgit.exe");

interface Commit {
  id: string;
  message: string;
  date: string;
  time: string;
  isHead: boolean;
}

function parseLogOutput(rawOutput: string): Commit[] {
  // Strip ANSI color escape codes
  const stripped = rawOutput.replace(/\x1b\[[0-9;]*m/g, "");

  const commits: Commit[] = [];
  // Split on the separator line
  const blocks = stripped.split("----------------------------------------").filter(b => b.trim());

  for (const block of blocks) {
    const lines = block.split("\n").map(l => l.trim()).filter(Boolean);
    if (lines.length === 0) continue;

    const commitLine = lines.find(l => l.startsWith("commit "));
    if (!commitLine) continue;

    const idMatch = commitLine.match(/commit\s+([a-fA-F0-9]+)/);
    if (!idMatch) continue;
    const id = idMatch[1];
    const isHead = commitLine.includes("HEAD");

    const dateLine = lines.find(l => l.startsWith("Date:"));
    const timeLine = lines.find(l => l.startsWith("Time:"));

    const date = dateLine ? dateLine.replace("Date:", "").trim() : "Unknown";
    const time = timeLine ? timeLine.replace("Time:", "").trim() : "";

    // Message is the line that's not id/date/time line
    const msgLine = lines.find(
      l =>
        !l.startsWith("commit ") &&
        !l.startsWith("Date:") &&
        !l.startsWith("Time:")
    );
    const message = msgLine || "No message";

    commits.push({ id, message, date, time, isHead });
  }

  return commits;
}

export async function GET() {
  try {
    const { stdout } = await execAsync(`"${MGIT_EXE}" log`, { cwd: REPO_PATH });
    const commits = parseLogOutput(stdout);
    return NextResponse.json({ commits });
  } catch (error: unknown) {
    const msgRaw = (error as {stdout?: string; message?: string}).stdout || (error as Error).message;
    const stripped = msgRaw?.replace(/\x1b\[[0-9;]*m/g, "") || "";
    if (stripped.includes("Not a MiniGit")) {
      return NextResponse.json({ commits: [], error: "Not a MiniGit repository" });
    }
    return NextResponse.json({ commits: [] });
  }
}
