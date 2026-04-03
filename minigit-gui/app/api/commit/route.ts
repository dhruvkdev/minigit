import { exec } from "child_process";
import { promisify } from "util";
import path from "path";
import { NextResponse, NextRequest } from "next/server";

const execAsync = promisify(exec);

const REPO_PATH = path.resolve(process.cwd(), "..");
const MGIT_EXE = path.join(REPO_PATH, "mgit.exe");

export async function POST(req: NextRequest) {
  try {
    const { message } = await req.json();
    if (!message || !message.trim()) {
      return NextResponse.json({ success: false, error: "Commit message is required." }, { status: 400 });
    }
    // Escape double quotes inside the message
    const safeMsg = message.trim().replace(/"/g, '\\"');
    const { stdout } = await execAsync(`"${MGIT_EXE}" commit "${safeMsg}"`, { cwd: REPO_PATH });
    const stripped = stdout.replace(/\x1b\[[0-9;]*m/g, "");
    if (stripped.includes("No files staged")) {
      return NextResponse.json({ success: false, error: "No files staged. Run Add first." });
    }
    return NextResponse.json({ success: true, output: stripped });
  } catch (error: unknown) {
    return NextResponse.json({ success: false, error: (error as Error).message }, { status: 500 });
  }
}
