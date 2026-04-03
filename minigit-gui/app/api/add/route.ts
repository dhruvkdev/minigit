import { exec } from "child_process";
import { promisify } from "util";
import path from "path";
import { NextResponse, NextRequest } from "next/server";

const execAsync = promisify(exec);

const REPO_PATH = path.resolve(process.cwd(), "..");
const MGIT_EXE = path.join(REPO_PATH, "mgit.exe");

export async function POST(req: NextRequest) {
  try {
    const { files } = await req.json() as { files: string[] };
    if (!files || files.length === 0) {
      return NextResponse.json({ success: false, error: "No files specified." }, { status: 400 });
    }
    const args = files.join(" ");
    const { stdout } = await execAsync(`"${MGIT_EXE}" add ${args}`, { cwd: REPO_PATH });
    return NextResponse.json({ success: true, output: stdout.trim() });
  } catch (error: unknown) {
    return NextResponse.json({ success: false, error: (error as Error).message }, { status: 500 });
  }
}
