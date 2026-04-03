import { exec } from "child_process";
import { promisify } from "util";
import path from "path";
import { NextResponse, NextRequest } from "next/server";

const execAsync = promisify(exec);

const REPO_PATH = path.resolve(process.cwd(), "..");
const MGIT_EXE = path.join(REPO_PATH, "mgit.exe");

// POST /api/branch { action: "create" | "checkout", name: string }
export async function POST(req: NextRequest) {
  try {
    const { action, name } = await req.json() as { action: string; name: string };
    if (!name || !name.trim()) {
      return NextResponse.json({ success: false, error: "Branch name required." }, { status: 400 });
    }

    let cmd = "";
    if (action === "create") {
      cmd = `"${MGIT_EXE}" branch "${name.trim()}"`;
    } else if (action === "checkout") {
      cmd = `"${MGIT_EXE}" checkout "${name.trim()}"`;
    } else {
      return NextResponse.json({ success: false, error: "Invalid action." }, { status: 400 });
    }

    const { stdout } = await execAsync(cmd, { cwd: REPO_PATH });
    const out = stdout.trim();
    if (out.toLowerCase().includes("does not exist")) {
      return NextResponse.json({ success: false, error: out });
    }
    return NextResponse.json({ success: true, output: out });
  } catch (error: unknown) {
    return NextResponse.json({ success: false, error: (error as Error).message }, { status: 500 });
  }
}
